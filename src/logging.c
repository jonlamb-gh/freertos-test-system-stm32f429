#include "FreeRTOS.h"

#include <stdarg.h>

#include "logging.h"

static TraceStringHandle_t g_info_ch = 0;
static TraceStringHandle_t g_warn_ch = 0;
static TraceStringHandle_t g_err_ch = 0;

void logging_init(void)
{
    traceResult tr;

    tr = xTraceStringRegister("info", &g_info_ch);
    configASSERT(tr == TRC_SUCCESS);

    tr = xTraceStringRegister("warn", &g_warn_ch);
    configASSERT(tr == TRC_SUCCESS);

    tr = xTraceStringRegister("error", &g_err_ch);
    configASSERT(tr == TRC_SUCCESS);
}

void logging_info(const char* fmt, ...)
{
    va_list vl;
    traceResult tr;

    if(xTraceIsRecorderEnabled() != 0)
    {
        va_start(vl, fmt);
        tr = xTraceVPrintF(g_info_ch, fmt, &vl);
        va_end(vl);
        configASSERT(tr == TRC_SUCCESS);
    }
}

void logging_warn(const char* fmt, ...)
{
    va_list vl;
    traceResult tr;

    if(xTraceIsRecorderEnabled() != 0)
    {
        va_start(vl, fmt);
        tr = xTraceVPrintF(g_warn_ch, fmt, &vl);
        va_end(vl);
        configASSERT(tr == TRC_SUCCESS);
    }
}

void logging_error(const char* fmt, ...)
{
    va_list vl;
    traceResult tr;

    if(xTraceIsRecorderEnabled() != 0)
    {
        va_start(vl, fmt);
        tr = xTraceVPrintF(g_err_ch, fmt, &vl);
        va_end(vl);
        configASSERT(tr == TRC_SUCCESS);
    }
}
