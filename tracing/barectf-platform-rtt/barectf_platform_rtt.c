#include <stdint.h>
#include <stdio.h>

#include "SEGGER_RTT.h"
#include "barectf_platform_rtt.h"

#if defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1)

#include "barectf.h"

#if defined(TRACE_CFG_USE_CRITICAL_SECTIONS)
#if !defined(TRACE_CFG_ALLOC_CRITICAL_SECTION) || !defined(TRACE_CFG_ENTER_CRITICAL_SECTION) || !defined(TRACE_CFG_EXIT_CRITICAL_SECTION)
    #error "Missing critical section definition. Define TRACE_CFG_ALLOC_CRITICAL_SECTION, TRACE_CFG_ENTER_CRITICAL_SECTION, and TRACE_CFG_EXIT_CRITICAL_SECTION in trace_config.h for your target."
#else
    #define TRACE_ALLOC_CRITICAL_SECTION() TRACE_CFG_ALLOC_CRITICAL_SECTION()
    #define TRACE_ENTER_CRITICAL_SECTION() TRACE_CFG_ENTER_CRITICAL_SECTION()
    #define TRACE_EXIT_CRITICAL_SECTION() TRACE_CFG_EXIT_CRITICAL_SECTION()
#endif
#else
    #define TRACE_ALLOC_CRITICAL_SECTION() TRACE_NOOP_STATEMENT
    #define TRACE_ENTER_CRITICAL_SECTION() TRACE_NOOP_STATEMENT
    #define TRACE_EXIT_CRITICAL_SECTION() TRACE_NOOP_STATEMENT
#endif /* defined(TRACE_CFG_USE_CRITICAL_SECTIONS) */

/* Use the 'default' clock type if TRACE_CFG_CLOCK_TYPE is not specified */
#if !defined(TRACE_CFG_CLOCK_TYPE)
#define TRACE_CFG_CLOCK_TYPE default
#endif

#if defined(TRACE_CFG_PACKET_CONTEXT_FIELD)
    #define bctf_open_packet(ctx, pctx) TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _open_packet)(ctx, pctx)
#else
    #define bctf_open_packet(ctx) TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _open_packet)(ctx)
#endif
#define bctf_close_packet(ctx) TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _close_packet)(ctx)

static barectf_stream_ctx g_barectf_stream_ctx;
static uint8_t g_pkt_buffer[TRACE_CFG_PACKET_SIZE] = {0};
static uint8_t g_rtt_up_buffer[TRACE_CFG_RTT_UP_BUFFER_SIZE] = {0};

static TRACE_CFG_CLOCK_C_TYPE get_clock(void* data)
{
    (void) data;
    return barectf_platform_rtt_get_clock();
}

static int is_backend_full(void* data)
{
    (void) data;

    const unsigned int available_bytes = SEGGER_RTT_GetAvailWriteSpace(TRACE_CFG_RTT_UP_BUFFER_INDEX);
    return available_bytes < TRACE_CFG_PACKET_SIZE;
}

static void open_packet(void* data)
{
    (void) data;

#if defined(TRACE_CFG_PACKET_CONTEXT_FIELD)
    bctf_open_packet(&g_barectf_stream_ctx, TRACE_CFG_PACKET_CONTEXT_FIELD);
#else
    bctf_open_packet(&g_barectf_stream_ctx);
#endif
}

static void close_packet(void* data)
{
    (void) data;

    /* Write the CTF packet data to the pkt */
    bctf_close_packet(&g_barectf_stream_ctx);

    /* Copy the pkt buffer to RTT channel buffer */
    (void) SEGGER_RTT_Write(
            TRACE_CFG_RTT_UP_BUFFER_INDEX,
            barectf_packet_buf(&g_barectf_stream_ctx),
            TRACE_CFG_PACKET_SIZE);
}

static void flush_or_close(int is_flush)
{
    TRACE_ALLOC_CRITICAL_SECTION();

    if(barectf_platform_rtt_is_enabled() != 0)
    {
        TRACE_ENTER_CRITICAL_SECTION();

        /* If there's a packet currently open, close it to write it out to the backend RTT buffer */
        const int pkt_is_open = barectf_packet_is_open(&g_barectf_stream_ctx);
        if(pkt_is_open != 0)
        {
            close_packet(NULL);

            if(is_flush != 0)
            {
                open_packet(NULL);
            }
        }

        TRACE_EXIT_CRITICAL_SECTION();
    }
}

void barectf_platform_rtt_init(void)
{
    TRACE_ALLOC_CRITICAL_SECTION();

    if(barectf_platform_rtt_is_enabled() == 0)
    {
        TRACE_ENTER_CRITICAL_SECTION();

        const int status = SEGGER_RTT_ConfigUpBuffer(
                TRACE_CFG_RTT_UP_BUFFER_INDEX,
                "barectf-rtt-up",
                &g_rtt_up_buffer[0],
                sizeof(g_rtt_up_buffer),
                SEGGER_RTT_MODE_NO_BLOCK_SKIP);
        TRACE_ASSERT(status == 0);

        struct barectf_platform_callbacks cbs =
        {
            .TRACE_CAT3(TRACE_CFG_CLOCK_TYPE, _clock_, get_value) = get_clock,
            .is_backend_full = is_backend_full,
            .open_packet = open_packet,
            .close_packet = close_packet,
        };

        barectf_init(&g_barectf_stream_ctx, &g_pkt_buffer[0], TRACE_CFG_PACKET_SIZE, cbs, NULL);

        open_packet(NULL);

        TRACE_EXIT_CRITICAL_SECTION();
    }
}

void barectf_platform_rtt_fini(void)
{
    flush_or_close(0 /* is_flush */);
}

int barectf_platform_rtt_is_enabled(void)
{
    return barectf_is_tracing_enabled(&g_barectf_stream_ctx);
}

void barectf_platform_rtt_flush(void)
{
    flush_or_close(1 /* is_flush */);
}

barectf_stream_ctx* barectf_platform_rtt_ctx(void)
{
    return &g_barectf_stream_ctx;
}

#else
/* Prevent diagnostic warning about empty translation unit when tracing is disabled */
typedef int make_the_compiler_happy;
#endif /* defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1) */
