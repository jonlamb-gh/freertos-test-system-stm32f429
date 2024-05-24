#include "FreeRTOS.h"

#include "stats_timer.h"

#include "stm32f4xx_hal_conf.h"

static TIM_HandleTypeDef g_timer = {0};

// Configure TIM2 (32-bit) to run at 100 KHz
void stats_timer_init(void)
{
    HAL_StatusTypeDef ret;

    __HAL_RCC_TIM2_CLK_ENABLE();

    g_timer.Instance = TIM2;
    g_timer.Init.Prescaler = (2 * 45000000/100000 - 1);
    g_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    g_timer.Init.Period = 0xFFFFFFFF;
    g_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    g_timer.Init.RepetitionCounter = 0;
    g_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    ret = HAL_TIM_Base_Init(&g_timer);
    configASSERT(ret == HAL_OK);

    ret = HAL_TIM_Base_Start(&g_timer);
    configASSERT(ret == HAL_OK);
}

uint32_t stats_timer_read(void)
{
    return g_timer.Instance->CNT;
}
