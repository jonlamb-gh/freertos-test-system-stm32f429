#include <stdint.h>
#include <stdio.h>

#include "system_trace_config.h"
#include "system_trace.h"
#include "barectf_platform_rtt.h"

#define TRC_REG_DWT_CYCCNT (*(volatile uint32_t*)0xE0001004)

TRACE_CFG_CLOCK_C_TYPE barectf_platform_rtt_get_clock(void)
{
    return TRC_REG_DWT_CYCCNT;
}

void system_trace_init(void)
{
    // TODO: we lean on trace recorder to setup the DWT clock...

    barectf_platform_rtt_init();
}
