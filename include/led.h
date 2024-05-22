#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    LED_GREEN = 0,
    LED_BLUE = 1,
    LED_RED = 2,
    LED_ALL = 3,
} led_kind;

void led_init(void);

void led_on(led_kind led);

void led_off(led_kind led);

void led_toggle(led_kind led);

#ifdef __cplusplus
}
#endif

#endif /* LED_H */
