// TODO
// NOTE: do not include this in header files

#ifndef TRACEPOINT_H
#define TRACEPOINT_H

// TODO this can be optional...
// this stuff can be defined manually in a tp.h, or a config.h or compiler options
//#if !defined(TRACEPOINT_CONFIG_INCLUDE)
//#error "TRACEPOINT_CONFIG_INCLUDE must be defined to the path to tracepoint_config.h"
//#endif

#include <stdint.h>

#include "system_trace_config.h"
#if defined(TRACEPOINT_CONFIG_INCLUDE)
#include TRACEPOINT_CONFIG_INCLUDE
#endif
#include "barectf_platform_rtt.h"
#include "trace_schema/barectf.h"

#if !defined(TRACEPOINT_COMPONENT)
#error "TRACEPOINT_COMPONENT must be defined to one of the trace-schema stream type names"
#endif

#if !defined(TRACE_CFG_PACKET_SIZE) || (TRACE_CFG_PACKET_SIZE <= 0)
#error "TRACE_CFG_PACKET_SIZE must be defined and be greater than zero"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* TODO docs */
#if !defined(TRACE_CFG_TRACEPOINT_ENABLED)
#define TRACE_CFG_TRACEPOINT_ENABLED TRACE_CFG_TRACING_ENABLED
#endif

#if defined(TRACE_CFG_TRACEPOINT_ENABLED) && (TRACE_CFG_TRACEPOINT_ENABLED != 0)

/* TODO docs */
#define TRACE_CFG_STREAM_TYPE TRACEPOINT_COMPONENT

/* TODO docs */
#if defined(TRACE_CFG_ENABLE_FROM_ISR) && (TRACE_CFG_ENABLE_FROM_ISR != 0)
#if !defined(TRACE_CFG_USE_CS) || (TRACE_CFG_USE_CS == 0)
#error "TRACE_CFG_ENABLE_FROM_ISR requires config TRACE_CFG_USE_CS to be enabled"
#endif
#endif /* defined(TRACE_CFG_ENABLE_FROM_ISR) */

/* TODO docs */
// TODO make TRACE_CFG_PACKET_CONTEXT_FIELD a fn-like macro ? FreeRTOS has pxCurrentTCB data...
#if defined(TRACE_CFG_PACKET_CONTEXT_FIELD)
    #define bctf_open_packet(ctx, pctx) TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _open_packet)(ctx, pctx)
#else
    #define bctf_open_packet(ctx) TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _open_packet)(ctx)
#endif
#define bctf_close_packet(ctx) TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _close_packet)(ctx)

/* TODO docs */
#if !defined(TRACEPOINT_CREATE_PROBE)
extern struct TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _ctx) TRACE_CAT3(g_barectf_, TRACE_CFG_STREAM_TYPE, _stream_ctx);
#endif

/* TODO docs */
void TRACE_CAT2(tracepoint_init_, TRACE_CFG_STREAM_TYPE)(void);
void TRACE_CAT2(tracepoint_fini_, TRACE_CFG_STREAM_TYPE)(void);
void TRACE_CAT2(tracepoint_flush_, TRACE_CFG_STREAM_TYPE)(void);
int TRACE_CAT2(tracepoint_is_enabled_, TRACE_CFG_STREAM_TYPE)(void);
void TRACE_CAT2(tracepoint_enable_, TRACE_CFG_STREAM_TYPE)(int enable);

#define tracepoint_init() TRACE_CAT2(tracepoint_init_, TRACE_CFG_STREAM_TYPE)()
#define tracepoint_fini() TRACE_CAT2(tracepoint_fini_, TRACE_CFG_STREAM_TYPE)()
#define tracepoint_flush() TRACE_CAT2(tracepoint_flush_, TRACE_CFG_STREAM_TYPE)()
#define tracepoint_is_enabled() TRACE_CAT2(tracepoint_is_enabled_, TRACE_CFG_STREAM_TYPE)()
#define tracepoint_enable(x) TRACE_CAT2(tracepoint_enable_, TRACE_CFG_STREAM_TYPE)(x)

/* Preprocessor trick to count arguments. Inspired from sdt.h. */
# define _TRACE_DEPAREN_0()                         /* empty */
# define _TRACE_DEPAREN_1(a)                        , a
# define _TRACE_DEPAREN_2(a,b)                      , a, b
# define _TRACE_DEPAREN_3(a,b,c)                    , a, b, c
# define _TRACE_DEPAREN_4(a,b,c,d)                  , a, b, c, d
# define _TRACE_DEPAREN_5(a,b,c,d,e)                , a, b, c, d, e
# define _TRACE_DEPAREN_6(a,b,c,d,e,f)              , a, b, c, d, e, f
# define _TRACE_DEPAREN_7(a,b,c,d,e,f,g)            , a, b, c, d, e, f, g
# define _TRACE_DEPAREN_8(a,b,c,d,e,f,g,h)          , a, b, c, d, e, f, g, h
# define _TRACE_DEPAREN_9(a,b,c,d,e,f,g,h,i)        , a, b, c, d, e, f, g, h, i
# define _TRACE_DEPAREN_10(a,b,c,d,e,f,g,h,i,j)     , a, b, c, d, e, f, g, h, i, j
# define _TRACE_DEPAREN_11(a,b,c,d,e,f,g,h,i,j,k)   , a, b, c, d, e, f, g, h, i, j, k
# define _TRACE_DEPAREN_12(a,b,c,d,e,f,g,h,i,j,k,l)	, a, b, c, d, e, f, g, h, i, j, k, l

#define _TRACE_NARG(...) __TRACE_NARG(__VA_ARGS__, 12,11,10,9,8,7,6,5,4,3,2,1,0)
#define __TRACE_NARG(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12, N, ...) N

/* Preprocessor trick to extract the args, with separators */
#define _TRACE_ARGS(n, args) _TRACE_DEPAREN_##n args

/* Inner macro for the primary tracepoint macro, handles the args and probe */
#define _TRACEPOINT(event, n, args)\
    TRACE_CAT4(barectf_, TRACE_CFG_STREAM_TYPE, _trace_, event)( \
            & TRACE_CAT3(g_barectf_, TRACE_CFG_STREAM_TYPE, _stream_ctx) \
            _TRACE_ARGS(n, args))

/* tracepoint with variadic support for up to 12 arguments */
#if defined(TRACE_CFG_ENABLE_FROM_ISR)
#define tracepoint(event, ...) \
    { \
        TRACE_ALLOC_CS(); \
        TRACE_ENTER_CS(); \
        _TRACEPOINT(event, _TRACE_NARG(0, ##__VA_ARGS__), (__VA_ARGS__)); \
        TRACE_EXIT_CS(); \
    }
#else
#define tracepoint(event, ...) \
    _TRACEPOINT(event, _TRACE_NARG(0, ##__VA_ARGS__), (__VA_ARGS__))
#endif /* defined(TRACE_CFG_ENABLE_FROM_ISR) */

/*
 * Create the definitions for the component probe and init functions.
 * This is usually done in the component-local tp.c source file.
 */
#if defined(TRACEPOINT_CREATE_PROBE)
#include <stdio.h>

struct TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _ctx) TRACE_CAT3(g_barectf_, TRACE_CFG_STREAM_TYPE, _stream_ctx) = {0};

static uint8_t g_pkt_buffer[TRACE_CFG_PACKET_SIZE] = {0};

static inline struct TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _ctx) * probe(void)
{
    return & TRACE_CAT3(g_barectf_, TRACE_CFG_STREAM_TYPE, _stream_ctx);
}

static TRACE_CFG_CLOCK_C_TYPE get_clock(void* data)
{
    (void) data;
    return barectf_platform_rtt_get_clock();
}

static int is_backend_full(void* data)
{
    (void) data;

    const unsigned int available_bytes = barectf_platform_rtt_available_write_size();
    return available_bytes < TRACE_CFG_PACKET_SIZE;
}

static void open_packet(void* data)
{
    (void) data;

#if defined(TRACE_CFG_PACKET_CONTEXT_FIELD)
    bctf_open_packet(probe(), TRACE_CFG_PACKET_CONTEXT_FIELD);
#else
    bctf_open_packet(probe());
#endif
}

static void close_packet(void* data)
{
    TRACE_ALLOC_CS();
    (void) data;

    /* Write the CTF packet data to the pkt */
    bctf_close_packet(probe());

    TRACE_ENTER_CS();

    /* Write it out */
    barectf_platform_rtt_write_packet(
            barectf_packet_buf(probe()),
            TRACE_CFG_PACKET_SIZE);

    TRACE_EXIT_CS();
}

static void flush_or_close(int is_flush)
{
    TRACE_ALLOC_CS();

    if(tracepoint_is_enabled() != 0)
    {
        TRACE_ENTER_CS();

        /* If there's a packet currently open, close it to write it out to the backend RTT buffer */
        const int pkt_is_open = barectf_packet_is_open(probe());
        if(pkt_is_open != 0)
        {
            close_packet(NULL);

            if(is_flush != 0)
            {
                open_packet(NULL);
            }
        }

        /* Disable tracing if closing */
        if(is_flush == 0)
        {
            barectf_enable_tracing(probe(), 0);
        }

        TRACE_EXIT_CS();
    }
}

void TRACE_CAT2(tracepoint_init_, TRACE_CFG_STREAM_TYPE)(void)
{
    TRACE_ALLOC_CS();

    if(tracepoint_is_enabled() == 0)
    {
        TRACE_ENTER_CS();

        struct barectf_platform_callbacks cbs =
        {
            .TRACE_CAT3(TRACE_CFG_CLOCK_TYPE, _clock_, get_value) = get_clock,
            .is_backend_full = is_backend_full,
            .open_packet = open_packet,
            .close_packet = close_packet,
        };

        barectf_init(probe(), &g_pkt_buffer[0], TRACE_CFG_PACKET_SIZE, cbs, NULL);

        open_packet(NULL);

        TRACE_EXIT_CS();
    }
}

void TRACE_CAT2(tracepoint_fini_, TRACE_CFG_STREAM_TYPE)(void)
{
    flush_or_close(0 /* is_flush */);
}

void TRACE_CAT2(tracepoint_flush_, TRACE_CFG_STREAM_TYPE)(void)
{
    flush_or_close(1 /* is_flush */);
}

int TRACE_CAT2(tracepoint_is_enabled_, TRACE_CFG_STREAM_TYPE)(void)
{
    return barectf_is_tracing_enabled(probe());
}

void TRACE_CAT2(tracepoint_enable_, TRACE_CFG_STREAM_TYPE)(int enable)
{
    TRACE_ALLOC_CS();
    TRACE_ENTER_CS();

    barectf_enable_tracing(probe(), enable);

    TRACE_EXIT_CS();
}
#endif /* defined(TRACEPOINT_CREATE_PROBE) */

#else /* defined(TRACE_CFG_TRACEPOINT_ENABLED) && (TRACE_CFG_TRACEPOINT_ENABLED != 0) */

#if !defined(TRACE_CFG_PACKET_SIZE)
#define TRACE_CFG_PACKET_SIZE (0)
#endif

#define tracepoint_init() TRACE_NOOP_STATEMENT
#define tracepoint_fini() TRACE_NOOP_STATEMENT
#define tracepoint_flush() TRACE_NOOP_STATEMENT
#define tracepoint_is_enabled() (0)
#define tracepoint_enable(x) TRACE_NOOP_STATEMENT
#define tracepoint(event, ...) TRACE_NOOP_STATEMENT

#if defined(TRACEPOINT_CREATE_PROBE)
/* Prevent diagnostic warning about empty translation unit when tracing is disabled */
typedef int make_the_compiler_happy;
#endif /* defined(TRACEPOINT_CREATE_PROBE) */

#endif /* defined(TRACE_CFG_TRACEPOINT_ENABLED) && (TRACE_CFG_TRACEPOINT_ENABLED != 0) */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRACEPOINT_H */
