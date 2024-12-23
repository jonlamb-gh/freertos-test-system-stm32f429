#include <stdint.h>
#include <stdio.h>

#include "SEGGER_RTT.h"
#include "barectf_platform_rtt.h"

#if defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1)

static uint8_t g_rtt_up_buffer[TRACE_CFG_RTT_UP_BUFFER_SIZE] = {0};

void barectf_platform_rtt_init(void)
{
    const int status = SEGGER_RTT_ConfigUpBuffer(                          
            TRACE_CFG_RTT_UP_BUFFER_INDEX,                                                 
            "barectf-rtt-up",                                                              
            &g_rtt_up_buffer[0],                                                           
            sizeof(g_rtt_up_buffer),                    
            SEGGER_RTT_MODE_NO_BLOCK_SKIP);             
    TRACE_ASSERT(status == 0);
}

unsigned int barectf_platform_rtt_available_write_size(void)
{
    return SEGGER_RTT_GetAvailWriteSpace(TRACE_CFG_RTT_UP_BUFFER_INDEX);
}

void barectf_platform_rtt_write_packet(uint8_t* pkt, uint32_t pkt_size)
{
    /* Copy the pkt buffer to RTT channel buffer */
    TRACE_ASSERT(pkt != NULL);
    TRACE_ASSERT(pkt_size != 0);
    (void) SEGGER_RTT_Write(
            TRACE_CFG_RTT_UP_BUFFER_INDEX,
            (void*) pkt,
            (unsigned int) pkt_size);
}

#else
/* Prevent diagnostic warning about empty translation unit when tracing is disabled */
typedef int make_the_compiler_happy;
#endif /* defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1) */
