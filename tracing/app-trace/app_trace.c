#include <stdint.h>
#include <stdio.h>

#include "SEGGER_RTT.h"
#include "app_trace.h"

#define TRC_REG_DWT_CYCCNT (*(volatile uint32_t*)0xE0001004)

barectf_stream_ctx* g_probe = NULL;

uint32_t barectf_platform_rtt_get_clock(void)
{
    return TRC_REG_DWT_CYCCNT;
}

void app_trace_init(void)
{
    // NOTE: we lean on trace recorder to setup the DWT clock...

    barectf_platform_rtt_init();

    g_probe = barectf_platform_rtt_ctx();
    configASSERT(g_probe != NULL);
}
