#ifndef BARECTF_PLATFORM_RTT_H
#define BARECTF_PLATFORM_RTT_H

#include <stdint.h>

/* User's configuration file */
#include "trace_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO - need to redo this for things with args
#define TRACE_NOOP_STATEMENT do {} while( 0 )

#if defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1)

/* A macro to wrap calling the barectf generated tracing functions in a critical section if enabled.
 * Also useful for turning tracing calls into no-op statements based on TRACE_CFG_TRACING_ENABLED */
#if defined(TRACE_CFG_USE_CRITICAL_SECTIONS)
#define TRACE(barectf_trace_call) \
    { \
        TRACE_ALLOC_CRITICAL_SECTION(); \
        TRACE_ENTER_CRITICAL_SECTION(); \
        barectf_trace_call; \
        TRACE_EXIT_CRITICAL_SECTION(); \
    }
#else
#define TRACE(barectf_trace_call) \
    { \
        barectf_trace_call; \
    }
#endif /* defined(TRACE_CFG_USE_CRITICAL_SECTIONS) */

/* Defines the RTT buffer to use for writing the trace data */
#if !defined(TRACE_CFG_RTT_UP_BUFFER_INDEX)
#define TRACE_CFG_RTT_UP_BUFFER_INDEX (2)
#endif

#if !defined(TRACE_CFG_CLOCK_C_TYPE)
#error "TRACE_CFG_CLOCK_C_TYPE must be defined to match the barectf default clock $c-type"
#endif

#if !defined(TRACE_CFG_PACKET_SIZE) || (TRACE_CFG_PACKET_SIZE <= 0)
#error "TRACE_CFG_PACKET_SIZE must be defined and greater than zero"
#endif

#if !defined(TRACE_CFG_RTT_UP_BUFFER_SIZE) || (TRACE_CFG_RTT_UP_BUFFER_SIZE <= TRACE_CFG_PACKET_SIZE)
#error "TRACE_CFG_RTT_UP_BUFFER_SIZE must be defined and greater than TRACE_CFG_PACKET_SIZE"
#endif

#if defined(TRACE_CFG_USE_TRACE_ASSERT) && (TRACE_CFG_USE_TRACE_ASSERT != 0)
#if !defined(TRACE_ASSERT)
#error "TRACE_ASSERT must be defined when TRACE_CFG_USE_TRACE_ASSERT != 0"
#endif
#else
#define TRACE_ASSERT(eval) TRACE_NOOP_STATEMENT
#endif /* defined(TRACE_CFG_USE_TRACE_ASSERT) && (TRACE_CFG_USE_TRACE_ASSERT != 0) */

#if !defined(TRACE_CAT3)
#define TRACE__CAT3(a, b, c) a##b##c
#define TRACE_CAT3(a, b, c) TRACE__CAT3(a, b, c)
#endif /* !defined(TRACE_CAT3) */

/* Use the 'default' stream type if TRACE_CFG_STREAM_TYPE is not specified */
#if !defined(TRACE_CFG_STREAM_TYPE)
#define TRACE_CFG_STREAM_TYPE default
#endif
typedef struct TRACE_CAT3(barectf_, TRACE_CFG_STREAM_TYPE, _ctx) barectf_stream_ctx;

/* User must implement this fn */
extern TRACE_CFG_CLOCK_C_TYPE barectf_platform_rtt_get_clock(void);

/* Must ensure the clock source is configured prior to calling. */
void barectf_platform_rtt_init(void);

void barectf_platform_rtt_fini(void);

/* A wrapper to barectf_is_tracing_enabled */
int barectf_platform_rtt_is_enabled(void);

/* Write the currently open packet, if any, to the RTT channel buffer. */
void barectf_platform_rtt_flush(void);

barectf_stream_ctx* barectf_platform_rtt_ctx(void);

#else /* defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1) */

#if !defined(TRACE_CFG_PACKET_SIZE)
#define TRACE_CFG_PACKET_SIZE (0)
#endif

typedef int barectf_stream_ctx;
#define barectf_platform_rtt_init(x) TRACE_NOOP_STATEMENT
#define barectf_platform_rtt_fini() TRACE_NOOP_STATEMENT
#define barectf_platform_rtt_is_enabled() 0
#define barectf_platform_rtt_flush() TRACE_NOOP_STATEMENT
#define barectf_platform_rtt_ctx() (NULL)

#define TRACE(barectf_trace_call) TRACE_NOOP_STATEMENT

#endif /* defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1) */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BARECTF_PLATFORM_RTT_H */
