#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "logging.h"
#include "can.h"
#include "task_worker0.h"

#include "stm32f4xx_hal_conf.h"

#define TASK_PRIO (tskIDLE_PRIORITY + 2)
#define TASK_STACK_SIZE (2 * configMINIMAL_STACK_SIZE)

#define CAN_FRAME_QUEUE_LENGTH (8)

static void worker_task(void* params);
static void stress_cpu_gcd(void);

static QueueHandle_t g_can_frame_queue = NULL;

void task_worker0_start(void)
{
    BaseType_t ret;

    g_can_frame_queue = xQueueCreate(CAN_FRAME_QUEUE_LENGTH, sizeof(can_rx_frame_s));
    configASSERT(g_can_frame_queue != NULL);
    vTraceSetQueueName(g_can_frame_queue, "worker0_queue");

    ret = xTaskCreate(
            worker_task,
            "Worker0",
            TASK_STACK_SIZE,
            NULL,
            TASK_PRIO,
            NULL);
    configASSERT(ret == pdPASS);
}

void task_worker0_do_work(const can_rx_frame_s *const frame)
{
    BaseType_t ret;

    configASSERT(frame != NULL);
    configASSERT(frame->header.IDE == CAN_ID_STD);
    configASSERT(frame->header.StdId == DO_WORK0_ID);
    ret = xQueueSend(g_can_frame_queue, frame, 0 /* non-blocking */);
    if(ret != pdTRUE)
    {
        ERR("Failed to enqueue worker0 CAN rx frame");
    }
}

static void worker_task(void* params)
{
    BaseType_t ret;
    can_rx_frame_s frame;
    (void) params;

    while(1)
    {
        ret = xQueueReceive(g_can_frame_queue, &frame, portMAX_DELAY);
        configASSERT(ret == pdTRUE);

        stress_cpu_gcd();
    }
}

// compute Greatest Common Divisor
// https://github.com/ColinIanKing/stress-ng/blob/master/stress-cpu.c#L222
static void stress_cpu_gcd(void)
{
	uint32_t i, gcd_sum = 0;
	uint64_t lcm_sum = 0;

	for (i = 0; i < 16384; i++) {
		volatile uint32_t a = i, b = i % (3 + (1997 ^ i));
		volatile uint64_t lcm = ((uint64_t)a * b);

		while (b != 0) {
			volatile uint32_t r = b;
			b = a % b;
			a = r;
		}
		if (a)
			lcm_sum += (lcm / a);
		gcd_sum += a;
        portNOP();
	}
}
