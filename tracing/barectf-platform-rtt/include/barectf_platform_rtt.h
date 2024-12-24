#ifndef BARECTF_PLATFORM_RTT_H
#define BARECTF_PLATFORM_RTT_H

#include <stdint.h>

/* User's configuration file */
#include "system_trace_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO - need to redo this for things with args to prevent unused warnings
#define TRACE_NOOP_STATEMENT do {} while( 0 )

#if defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1)

#if defined(TRACE_CFG_USE_CS)
#if !defined(TRACE_CFG_ALLOC_CS) || !defined(TRACE_CFG_ENTER_CS) || !defined(TRACE_CFG_EXIT_CS)
    #error "Missing critical section definition. Define TRACE_CFG_ALLOC_CS, TRACE_CFG_ENTER_CS, and TRACE_CFG_EXIT_CS in system_trace_config.h for your target."
#else
    #define TRACE_ALLOC_CS() TRACE_CFG_ALLOC_CS()
    #define TRACE_ENTER_CS() TRACE_CFG_ENTER_CS()
    #define TRACE_EXIT_CS() TRACE_CFG_EXIT_CS()
#endif
#else
    #define TRACE_ALLOC_CS() TRACE_NOOP_STATEMENT
    #define TRACE_ENTER_CS() TRACE_NOOP_STATEMENT
    #define TRACE_EXIT_CS() TRACE_NOOP_STATEMENT
#endif /* defined(TRACE_CFG_USE_CS) */

/* Defines the RTT buffer to use for writing the trace data */
#if !defined(TRACE_CFG_RTT_UP_BUFFER_INDEX)
#error "TRACE_CFG_RTT_UP_BUFFER_INDEX must be defined in system_trace_config.h"
#endif

#if !defined(TRACE_CFG_RTT_UP_BUFFER_SIZE) || (TRACE_CFG_RTT_UP_BUFFER_SIZE <= TRACE_CFG_PACKET_SIZE)
#error "TRACE_CFG_RTT_UP_BUFFER_SIZE must be defined and greater than TRACE_CFG_PACKET_SIZE"
#endif

#if !defined(TRACE_CFG_CLOCK_TYPE)
#error "TRACE_CFG_CLOCK_TYPE must be defined manualy in system_trace_config.h"
#endif

#if !defined(TRACE_CFG_CLOCK_C_TYPE)
#error "TRACE_CFG_CLOCK_C_TYPE must be defined manualy in system_trace_config.h"
#endif

#if defined(TRACE_CFG_USE_TRACE_ASSERT) && (TRACE_CFG_USE_TRACE_ASSERT != 0)
#if !defined(TRACE_ASSERT)
#error "TRACE_ASSERT must be defined when TRACE_CFG_USE_TRACE_ASSERT != 0"
#endif
#else
#define TRACE_ASSERT(eval) TRACE_NOOP_STATEMENT
#endif /* defined(TRACE_CFG_USE_TRACE_ASSERT) && (TRACE_CFG_USE_TRACE_ASSERT != 0) */

/* A low-level macro to wrap calling the barectf generated tracing functions in a critical section if enabled.
 * Also useful for turning tracing calls into no-op statements based on TRACE_CFG_TRACING_ENABLED */
#if defined(TRACE_CFG_USE_CS)
#define TRACE(barectf_trace_call) \
    { \
        TRACE_ALLOC_CS(); \
        TRACE_ENTER_CS(); \
        barectf_trace_call; \
        TRACE_EXIT_CS(); \
    }
#else
#define TRACE(barectf_trace_call) \
    { \
        barectf_trace_call; \
    }
#endif /* defined(TRACE_CFG_USE_CS) */

#if !defined(TRACE_CAT2)
#define TRACE__CAT2(a, b) a##b
#define TRACE_CAT2(a, b) TRACE__CAT2(a, b)
#endif /* !defined(TRACE_CAT3) */

#if !defined(TRACE_CAT3)
#define TRACE__CAT3(a, b, c) a##b##c
#define TRACE_CAT3(a, b, c) TRACE__CAT3(a, b, c)
#endif /* !defined(TRACE_CAT3) */

#if !defined(TRACE_CAT4)
#define TRACE__CAT4(a, b, c, d) a##b##c##d
#define TRACE_CAT4(a, b, c, d) TRACE__CAT4(a, b, c, d)
#endif /* !defined(TRACE_CAT3) */

/* User must implement this fn */
extern TRACE_CFG_CLOCK_C_TYPE barectf_platform_rtt_get_clock(void);

void barectf_platform_rtt_init(void);

unsigned int barectf_platform_rtt_available_write_size(void);

void barectf_platform_rtt_write_packet(uint8_t* pkt, uint32_t pkt_size);

#else /* defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1) */

#define barectf_platform_rtt_init(x) TRACE_NOOP_STATEMENT
#define barectf_platform_rtt_available_write_size() TRACE_NOOP_STATEMENT
#define barectf_platform_rtt_write_packet(x, y) TRACE_NOOP_STATEMENT

#define TRACE(barectf_trace_call) TRACE_NOOP_STATEMENT

#endif /* defined(TRACE_CFG_TRACING_ENABLED) && (TRACE_CFG_TRACING_ENABLED == 1) */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BARECTF_PLATFORM_RTT_H */
