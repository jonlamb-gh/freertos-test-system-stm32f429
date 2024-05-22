#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "led.h"
#include "logging.h"
#include "task_shell.h"
#include "task_can1.h"
#include "task_can2.h"

#include "stm32f4xx_hal_conf.h"

static void led_heartbeat_timer_cb(TimerHandle_t timer);
static void hw_init(void);
static void configure_system_clock(void);

int main(void)
{
    traceResult tr;

    hw_init();

    // Configure the system clock to 180 MHz
    configure_system_clock();

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
    task_can1_start();
    task_can2_start();

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

static void hw_init(void)
{
    // Configure Flash prefetch, Instruction cache, Data cache
#if (INSTRUCTION_CACHE_ENABLE != 0U)
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
#endif /* INSTRUCTION_CACHE_ENABLE */

#if (DATA_CACHE_ENABLE != 0U)
    __HAL_FLASH_DATA_CACHE_ENABLE();
#endif /* DATA_CACHE_ENABLE */

#if (PREFETCH_ENABLE != 0U)
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
#endif /* PREFETCH_ENABLE */

    // NOTE: startup_stm32f429xx.s calls SystemInit() before main
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

/*
 * The system Clock is configured as follow : 
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 180000000
 *            HCLK(Hz)                       = 180000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 8000000
 *            PLL_M                          = 8
 *            PLL_N                          = 360
 *            PLL_P                          = 2
 *            PLL_Q                          = 7
 *            PLL_R                          = 2
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 5
 */
static void configure_system_clock(void)
{
    HAL_StatusTypeDef ret;
    RCC_ClkInitTypeDef clk_init;
    RCC_OscInitTypeDef osc_init;

    // Enable Power Control clock
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // Enable HSE Oscillator and activate PLL with HSE as source
    osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc_init.HSEState = RCC_HSE_BYPASS;
    osc_init.PLL.PLLState = RCC_PLL_ON;
    osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc_init.PLL.PLLM = 8;
    osc_init.PLL.PLLN = 360;
    osc_init.PLL.PLLP = RCC_PLLP_DIV2;
    osc_init.PLL.PLLQ = 7;
    ret = HAL_RCC_OscConfig(&osc_init);
    configASSERT(ret == HAL_OK);

    ret = HAL_PWREx_EnableOverDrive();
    configASSERT(ret == HAL_OK);

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    clk_init.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk_init.APB1CLKDivider = RCC_HCLK_DIV4;
    clk_init.APB2CLKDivider = RCC_HCLK_DIV2;
    ret = HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_5);
    configASSERT(ret == HAL_OK);
}

// TODO: timer/etc for this, timeouts will never get hit currently
// Should be able to rm it once on the LL APIs
uint32_t uwTickPrio = (1UL << __NVIC_PRIO_BITS); /* Invalid PRIO */
uint32_t HAL_GetTick(void)
{
    return 0;
}
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    uwTickPrio = TickPriority;
    return HAL_OK;
}

