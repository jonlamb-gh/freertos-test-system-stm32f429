#ifndef APP_TRACE_H
#define APP_TRACE_H

#include "barectf_platform_rtt.h"
#include "barectf.h"

#ifdef __cplusplus
extern "C" {
#endif

extern barectf_stream_ctx* g_probe;

void app_trace_init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_TRACE_H */
