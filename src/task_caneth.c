#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include <string.h> /* memcpy */

#include "logging.h"
#include "can.h"
#include "caneth.h"
#include "task_caneth.h"

#include "stm32f4xx_hal_conf.h"

#define TASK_PRIO (tskIDLE_PRIORITY + 2)
#define TASK_STACK_SIZE (2 * configMINIMAL_STACK_SIZE)

#define FLUSH_TIMEOUT pdMS_TO_TICKS(100)

#define CAN_FRAME_QUEUE_LENGTH 32

#define QUEUE_EVENT_BIT (0x01)
#define FLUSH_EVENT_BIT (0x02)
#define ALL_EVENTS 0xFFFFFFFF

static void caneth_task(void* params);
static void flush_timer_cb(TimerHandle_t timer);

static TaskHandle_t g_caneth_task = NULL;
static QueueHandle_t g_can_frame_queue = NULL;
static TimerHandle_t g_flush_timer = NULL;

void task_caneth_start(void)
{
    BaseType_t ret;

    g_can_frame_queue = xQueueCreate(CAN_FRAME_QUEUE_LENGTH, sizeof(caneth_frame_s));
    configASSERT(g_can_frame_queue != NULL);
    vTraceSetQueueName(g_can_frame_queue, "can_frame_queue");

    ret = xTaskCreate(
            caneth_task,
            "CAN_ETH",
            TASK_STACK_SIZE,
            NULL,
            TASK_PRIO,
            &g_caneth_task);
    configASSERT(ret == pdPASS);

    g_flush_timer = xTimerCreate(
            "caneth_flush_timer",
            FLUSH_TIMEOUT,
            pdTRUE, // auto-reload
            NULL,
            flush_timer_cb);
    configASSERT(g_flush_timer != NULL);
    xTimerStart(g_flush_timer, 0);
}

void task_caneth_enqueue_rx_frame(const can_rx_frame_s *const frame)
{
    BaseType_t ret;
    
    if(FreeRTOS_IsNetworkUp() == pdTRUE)
    {
        caneth_frame_s qframe = {0};
        configASSERT(frame != NULL);
        qframe.id = (frame->header.IDE == CAN_ID_STD) ? frame->header.StdId : frame->header.ExtId;
        qframe.count = frame->header.DLC;
        qframe.ext_flag = (frame->header.IDE == CAN_ID_EXT);
        qframe.rtr_flag = (frame->header.RTR == CAN_RTR_REMOTE);
        (void) memcpy(qframe.data, frame->data, sizeof(qframe.data));

        ret = xQueueSend(g_can_frame_queue, &qframe, 0 /* non-blocking */);
        if(ret == pdTRUE)
        {
            ret = xTaskNotify(g_caneth_task, QUEUE_EVENT_BIT, eSetBits);
            configASSERT(ret == pdPASS);
        }
        else
        {
            ERR("Failed to enqueue CANETH rx frame");
        }
    }
}

static void caneth_task(void* params)
{
    uint32_t notif_bits;
    BaseType_t ret;
    uint8_t* udp_buffer;
    uint16_t udp_seqnum = 0;
    uint8_t buffered_frames = 0;
    Socket_t socket = FREERTOS_INVALID_SOCKET;
    struct freertos_sockaddr addr = {0};
    (void) params;

    INFO("Waiting for network up");
    while(FreeRTOS_IsNetworkUp() == pdFALSE)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // Broadcast on the CANETH UDP port
    addr.sin_port = FreeRTOS_htons(CANETH_UDP_PORT);
    addr.sin_addr = FreeRTOS_inet_addr_quick(
            configIP_ADDR0,
            configIP_ADDR1,
            configIP_ADDR2,
            255);

    socket = FreeRTOS_socket(
            FREERTOS_AF_INET,
            FREERTOS_SOCK_DGRAM,
            FREERTOS_IPPROTO_UDP);
    configASSERT(socket != FREERTOS_INVALID_SOCKET);

    INFO("Waiting for initial UDP buffer");
    do
    {
        udp_buffer = (uint8_t*) FreeRTOS_GetUDPPayloadBuffer(
                CANETH_DATAGRAM_MAX_SIZE,
                portMAX_DELAY);
    }
    while(udp_buffer == NULL);

    INFO("Ready");

    while(1)
    {
        // Wait for a queue or timer event
        ret = xTaskNotifyWait(
                pdFALSE, // Don't clear bits on entry
                ALL_EVENTS, // Clear all bits on exit
                &notif_bits,
                portMAX_DELAY);
        configASSERT(ret == pdTRUE);

        if((notif_bits & QUEUE_EVENT_BIT) != 0)
        {
            caneth_frame_s *const frame_buffer = (caneth_frame_s*) &udp_buffer[sizeof(caneth_header_s)];
            configASSERT(buffered_frames < CANETH_MAX_COUNT);
            ret = xQueueReceive(g_can_frame_queue, &frame_buffer[buffered_frames], 0 /* non-blocking */);
            if(ret == pdTRUE)
            {
                // Should always happen since we we're woken up via notification
                buffered_frames += 1;
            }
        }

        const int ready_to_send = (buffered_frames == CANETH_MAX_COUNT) || ((notif_bits & FLUSH_EVENT_BIT) != 0);
        if((ready_to_send != 0) && (buffered_frames != 0))
        {
            udp_seqnum += 1;

            caneth_header_s *const header = (caneth_header_s*) udp_buffer;
            header->magic_id = CANETH_MAGIC_ID;
            header->version = CANETH_VERSION;
            header->seqnum = udp_seqnum;
            header->count = buffered_frames;

            const size_t udp_size = sizeof(caneth_header_s) + (sizeof(caneth_frame_s) * buffered_frames);
            ret = FreeRTOS_sendto(socket, udp_buffer, udp_size, FREERTOS_ZERO_COPY, &addr, sizeof(addr));
            if(ret <= 0)
            {
                ERR("Failed to send CANETH udp");
                FreeRTOS_ReleaseUDPPayloadBuffer((void*) udp_buffer);
            }

            // Favor recency, will drop buffered frames if they weren't sent
            buffered_frames = 0;

            // Request another zero-copy buffer
            do
            {
                udp_buffer = (uint8_t*) FreeRTOS_GetUDPPayloadBuffer(
                        CANETH_DATAGRAM_MAX_SIZE,
                        portMAX_DELAY);
            }
            while(udp_buffer == NULL);
        }
    }
}

static void flush_timer_cb(TimerHandle_t timer)
{
    BaseType_t ret;
    (void) timer;

    configASSERT(g_caneth_task != NULL);
    ret = xTaskNotify(g_caneth_task, FLUSH_EVENT_BIT, eSetBits);
    configASSERT(ret == pdPASS);
}
