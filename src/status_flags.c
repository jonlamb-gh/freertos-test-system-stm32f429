#include "FreeRTOS.h"
#include "atomic.h"

#include "status_flags.h"

static volatile uint32_t g_can1_error = 0;
static volatile uint32_t g_can2_error = 0;
static volatile uint32_t g_caneth_error = 0;
static volatile uint32_t g_shell_error = 0;

void status_flags_set_can1_error(void)
{
    Atomic_OR_u32(&g_can1_error, 1);
}

uint8_t status_flags_get_can1_error(void)
{
    return (uint8_t) g_can1_error;
}

void status_flags_set_can2_error(void)
{
    Atomic_OR_u32(&g_can2_error, 1);
}

uint8_t status_flags_get_can2_error(void)
{
    return (uint8_t) g_can2_error;
}

void status_flags_set_caneth_error(void)
{
    Atomic_OR_u32(&g_caneth_error, 1);
}

uint8_t status_flags_get_caneth_error(void)
{
    return (uint8_t) g_caneth_error;
}

void status_flags_set_shell_error(void)
{
    Atomic_OR_u32(&g_shell_error, 1);
}

uint8_t status_flags_get_shell_error(void)
{
    return (uint8_t) g_shell_error;
}
