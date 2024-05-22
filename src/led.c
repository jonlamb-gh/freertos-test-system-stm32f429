#include "led.h"

#include "stm32f4xx_hal_conf.h"

// active-high
#define LED_PORT GPIOB
#define LED_GREEN_PIN GPIO_PIN_0
#define LED_BLUE_PIN GPIO_PIN_7
#define LED_RED_PIN GPIO_PIN_14

static uint16_t led_pin(led_kind led)
{
    uint16_t pin = 0;
    if(led == LED_GREEN)
    {
        pin = LED_GREEN_PIN;
    }
    else if(led == LED_BLUE)
    {
        pin = LED_BLUE_PIN;
    }
    else if(led == LED_RED)
    {
        pin = LED_RED_PIN;
    }
    else if(led == LED_ALL)
    {
        pin = LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN;
    }
    else
    {
        assert_param(0);
    }
    return pin;
}

void led_init(void)
{
    GPIO_InitTypeDef gpio_init =
    {
        .Pin = (LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN),
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_MEDIUM,
        .Alternate = 0,
    };

    __HAL_RCC_GPIOB_CLK_ENABLE();
    HAL_GPIO_Init(LED_PORT, &gpio_init);

    HAL_GPIO_WritePin(LED_PORT, LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN, GPIO_PIN_RESET);
}

void led_on(led_kind led)
{
    const uint16_t pin = led_pin(led);
    HAL_GPIO_WritePin(LED_PORT, pin, GPIO_PIN_SET);
}

void led_off(led_kind led)
{
    const uint16_t pin = led_pin(led);
    HAL_GPIO_WritePin(LED_PORT, pin, GPIO_PIN_RESET);
}

void led_toggle(led_kind led)
{
    const uint16_t pin = led_pin(led);
    HAL_GPIO_TogglePin(LED_PORT, pin);
}
