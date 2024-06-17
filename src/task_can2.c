#include "FreeRTOS.h"
#include "task.h"

#include "logging.h"
#include "led.h"
#include "can.h"
#include "task_can2.h"

#include "stm32f4xx_hal_conf.h"

#define TASK_PRIO (tskIDLE_PRIORITY + 4)
#define TASK_STACK_SIZE (2 * configMINIMAL_STACK_SIZE)

#define HEARTBEAT_TX_INTERVAL (pdMS_TO_TICKS(50))

#define CAN2_GPIO_PORT GPIOB
#define CAN2_RX_PIN GPIO_PIN_5
#define CAN2_TX_PIN GPIO_PIN_6

static void init_can2(void);
static void can2_task(void* params);
static void can2_error_cb(CAN_HandleTypeDef *hcan);

static CAN_HandleTypeDef g_can = {0};

void task_can2_start(void)
{
    BaseType_t ret;

    init_can2();

    ret = xTaskCreate(
            can2_task,
            "CAN2",
            TASK_STACK_SIZE,
            NULL,
            TASK_PRIO,
            NULL);
    configASSERT(ret == pdPASS);
}

static void init_can2(void)
{
    HAL_StatusTypeDef ret;
    CAN_FilterTypeDef filter_cfg;
    GPIO_InitTypeDef gpio_init =
    {
        .Pin = (CAN2_RX_PIN | CAN2_TX_PIN),
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = GPIO_AF9_CAN2,
    };

    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_Init(CAN2_GPIO_PORT, &gpio_init);

    // 500K
    g_can.Instance = CAN2;
    g_can.Init.TimeTriggeredMode = ENABLE;
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
    __HAL_RCC_CAN2_CLK_ENABLE();
    ret = HAL_CAN_Init(&g_can);
    configASSERT(ret == HAL_OK);

    ret = HAL_CAN_RegisterCallback(
            &g_can,
            HAL_CAN_ERROR_CB_ID,
            can2_error_cb);
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
}

static void can2_task(void* params)
{
    HAL_StatusTypeDef ret;
    TickType_t next_wake;
    uint32_t tx_mbox;
    can_tx_frame_s frame = {0};
    struct canproto_heartbeat_t hb_msg;
    (void) params;

    frame.header.StdId = CANPROTO_HEARTBEAT_FRAME_ID;
    frame.header.ExtId = 0x00;
    frame.header.RTR = CAN_RTR_DATA;
    frame.header.IDE = CAN_ID_STD;
    frame.header.DLC = CANPROTO_HEARTBEAT_LENGTH;
    frame.header.TransmitGlobalTime = ENABLE;

    ret = canproto_heartbeat_init(&hb_msg);
    configASSERT(ret == 0);

    next_wake = xTaskGetTickCount();
    while(1)
    {
        const BaseType_t was_delayed = xTaskDelayUntil(&next_wake, HEARTBEAT_TX_INTERVAL);
        if(was_delayed == pdFALSE)
        {
            WARN("Deadline missed");
        }

        hb_msg.seqnum += 1;
        ret = canproto_heartbeat_pack(&frame.data[0], &hb_msg, sizeof(frame.data));
        configASSERT(ret == CANPROTO_HEARTBEAT_LENGTH);

        ret = HAL_CAN_AddTxMessage(&g_can, &frame.header, &frame.data[0], &tx_mbox);
        if(ret != HAL_OK)
        {
            led_on(LED_RED);
        }
    }
}

static void can2_error_cb(CAN_HandleTypeDef *hcan)
{
    (void) hcan;

    led_on(LED_RED);
}
