#ifndef SYSTEM_TRACE_CONFIG_H
#define SYSTEM_TRACE_CONFIG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TRACE_CFG_TRACING_ENABLED (1)

/* Must match the barectf default clock $c-type field */
#define TRACE_CFG_CLOCK_C_TYPE uint32_t
#define TRACE_CFG_CLOCK_TYPE dwt_clock

#define TRACE_CFG_RTT_UP_BUFFER_INDEX (2)
#define TRACE_CFG_RTT_UP_BUFFER_SIZE (1024 * 1)

#define TRACE_CFG_USE_TRACE_ASSERT (1)
void vAssertCalled(const char* file, int line);
#define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled(__FILE__, __LINE__)
#define TRACE_ASSERT(eval) configASSERT(eval)

#ifndef __CORTEX_M
#define __CORTEX_M (4U)
#endif
#include "cmsis_gcc.h"
#define TRACE_CFG_USE_CS (1)
#define TRACE_CFG_ALLOC_CS_NAME xTraceCriticalSectionStatus
#define TRACE_CFG_ALLOC_CS() uint32_t TRACE_CFG_ALLOC_CS_NAME;
#define TRACE_CFG_ENTER_CS() {TRACE_CFG_ALLOC_CS_NAME = __get_PRIMASK(); __set_PRIMASK(1);} /* PRIMASK disables ALL interrupts - allows for tracing in any ISR */
#define TRACE_CFG_EXIT_CS() {__set_PRIMASK(TRACE_CFG_ALLOC_CS_NAME);}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SYSTEM_TRACE_CONFIG_H */
