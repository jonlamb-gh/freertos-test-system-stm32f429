#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "logging.h"
#include "led.h"
#include "can.h"
#include "task_caneth.h"
#include "task_worker0.h"
#include "task_can1.h"

#include "stm32f4xx_hal_conf.h"

#define TASK_PRIO (tskIDLE_PRIORITY + 3)
#define TASK_STACK_SIZE (2 * configMINIMAL_STACK_SIZE)
#define ISR_PRIO configLIBRARY_LOWEST_INTERRUPT_PRIORITY

#define CAN1_GPIO_PORT GPIOD
#define CAN1_RX_PIN GPIO_PIN_0
#define CAN1_TX_PIN GPIO_PIN_1

#define RX_MSGBUF_SIZE (sizeof(can_rx_frame_s) * 32)

static void init_can1(void);
static void can1_task(void* params);
static void can1_pending_cb(CAN_HandleTypeDef *hcan);
static void can1_error_cb(CAN_HandleTypeDef *hcan);

static MessageBufferHandle_t g_rx_msgbuf = NULL;
static CAN_HandleTypeDef g_can = {0};
static TraceISRHandle_t g_isr_handle = NULL;

void task_can1_start(void)
{
    BaseType_t ret;
    traceResult tr;

    tr = xTraceISRRegister("CAN1_RX0", ISR_PRIO, &g_isr_handle);
    configASSERT(tr == TRC_SUCCESS);

    init_can1();

    g_rx_msgbuf = xMessageBufferCreate(RX_MSGBUF_SIZE);
    configASSERT(g_rx_msgbuf != NULL);
    vTraceSetMessageBufferName(g_rx_msgbuf, "can1_rx_msg_buf");

    ret = xTaskCreate(
            can1_task,
            "CAN1",
            TASK_STACK_SIZE,
            NULL,
            TASK_PRIO,
            NULL);
    configASSERT(ret == pdPASS);
}

static void init_can1(void)
{
    HAL_StatusTypeDef ret;
    CAN_FilterTypeDef filter_cfg;
    GPIO_InitTypeDef gpio_init =
    {
        .Pin = (CAN1_RX_PIN | CAN1_TX_PIN),
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = GPIO_AF9_CAN1,
    };

    __HAL_RCC_GPIOD_CLK_ENABLE();
    HAL_GPIO_Init(CAN1_GPIO_PORT, &gpio_init);

    NVIC_SetPriority(CAN1_RX0_IRQn, ISR_PRIO);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    // 500K
    g_can.Instance = CAN1;
    g_can.Init.TimeTriggeredMode = DISABLE;
    g_can.Init.AutoBusOff = DISABLE;
    g_can.Init.AutoWakeUp = DISABLE;
    g_can.Init.AutoRetransmission = ENABLE;
    g_can.Init.ReceiveFifoLocked = DISABLE;
    g_can.Init.TransmitFifoPriority = DISABLE;
    g_can.Init.Mode = CAN_MODE_NORMAL;
    g_can.Init.SyncJumpWidth = CAN_SJW_1TQ;
    g_can.Init.TimeSeg1 = CAN_BS1_15TQ;
    g_can.Init.TimeSeg2 = CAN_BS2_2TQ;
    g_can.Init.Prescaler = 5;
    __HAL_RCC_CAN1_CLK_ENABLE();
    ret = HAL_CAN_Init(&g_can);
    configASSERT(ret == HAL_OK);

    ret = HAL_CAN_RegisterCallback(
            &g_can,
            HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID,
            can1_pending_cb);
    configASSERT(ret == HAL_OK);
    ret = HAL_CAN_RegisterCallback(
            &g_can,
            HAL_CAN_ERROR_CB_ID,
            can1_error_cb);
    configASSERT(ret == HAL_OK);

    filter_cfg.FilterBank = 0;
    filter_cfg.FilterMode = CAN_FILTERMODE_IDMASK;
    filter_cfg.FilterScale = CAN_FILTERSCALE_32BIT;
    filter_cfg.FilterIdHigh = 0x0000;
    filter_cfg.FilterIdLow = 0x0000;
    filter_cfg.FilterMaskIdHigh = 0x0000;
    filter_cfg.FilterMaskIdLow = 0x0000;
    filter_cfg.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter_cfg.FilterActivation = ENABLE;
    filter_cfg.SlaveStartFilterBank = 14;
    ret = HAL_CAN_ConfigFilter(&g_can, &filter_cfg);
    configASSERT(ret == HAL_OK);

    ret = HAL_CAN_Start(&g_can);
    configASSERT(ret == HAL_OK);

    ret = HAL_CAN_ActivateNotification(&g_can, CAN_IT_RX_FIFO0_MSG_PENDING);
    configASSERT(ret == HAL_OK);
}

static void can1_task(void* params)
{
    can_rx_frame_s rx_frame;
    (void) params;

    while(1)
    {
        // Wait for data
        while(xMessageBufferReceive(g_rx_msgbuf, &rx_frame, sizeof(rx_frame), portMAX_DELAY) == 0);

        // Send it to the CAN-to-ETH task
        task_caneth_enqueue_rx_frame(&rx_frame);

        // Dispatch to various workers
        if(rx_frame.header.StdId == DO_WORK0_ID)
        {
            task_worker0_do_work(&rx_frame);
        }
    }
}

static void can1_pending_cb(CAN_HandleTypeDef *hcan)
{
    HAL_StatusTypeDef ret;
    size_t bytes_sent;
    BaseType_t higher_prio_task_woken;
    static can_rx_frame_s rx_frame;

    led_toggle(LED_BLUE);

    ret = HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_frame.header, &rx_frame.data[0]);
    if(ret == HAL_OK)
    {
        bytes_sent = xMessageBufferSendFromISR(
                g_rx_msgbuf,
                &rx_frame,
                sizeof(rx_frame),
                &higher_prio_task_woken);
        if(bytes_sent == 0)
        {
            led_on(LED_RED);
        }
        portYIELD_FROM_ISR(higher_prio_task_woken);
    }
    else
    {
        led_on(LED_RED);
    }
}

static void can1_error_cb(CAN_HandleTypeDef *hcan)
{
    (void) hcan;

    led_on(LED_RED);
}

void CAN1_RX0_IRQHandler(void)
{
    traceResult tr;

    tr = xTraceISRBegin(g_isr_handle);
    configASSERT(tr == TRC_SUCCESS);

    HAL_CAN_IRQHandler(&g_can);

    tr = xTraceISREnd(0);
    configASSERT(tr == TRC_SUCCESS);
}
