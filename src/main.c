#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "led.h"
#include "logging.h"
#include "task_shell.h"

#include "stm32f4xx_hal_conf.h"

static void led_heartbeat_timer_cb(TimerHandle_t timer);

int main(void)
{
    traceResult tr;

    /* NOTE: startup_stm32f429xx.s calls SystemInit() before main */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // TODO - crank up the clocks

    led_init();
    led_on(LED_GREEN);

    // TODO - just initialize, start from shell
    tr = xTraceEnable(TRC_START);
    configASSERT(tr == TRC_SUCCESS);

    logging_init();

    TimerHandle_t timer = xTimerCreate("led_heartbeat_timer", pdMS_TO_TICKS(500), pdTRUE, NULL, led_heartbeat_timer_cb);
    configASSERT(timer != NULL);
    xTimerStart(timer, 0);

    task_shell_start();

    configASSERT(xTraceDiagnosticsCheckStatus() == TRC_SUCCESS);
    const char* err = NULL;
    (void) xTraceErrorGetLast(&err);
    if(err != NULL)
    {
        ERR("Trace error: %s", err);
    }
    configASSERT(err == NULL);

    INFO("Starting scheduler");
    vTaskStartScheduler();

    while(1)
    {
        portNOP();
    }

    return 0;
}

static void led_heartbeat_timer_cb(TimerHandle_t timer)
{
    (void) timer;
    led_toggle(LED_GREEN);
}

/* TODO: timer/etc for this, timeouts will never get hit currently */
// Should be able to rm it once on the LL APIs
/*
uint32_t HAL_GetTick(void)
{
    return 0;
}
*/
