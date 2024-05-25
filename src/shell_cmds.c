// More examples:
// https://github.com/FreeRTOS/FreeRTOS/blob/main/FreeRTOS-Plus/Demo/Common/FreeRTOS_Plus_CLI_Demos/Sample-CLI-commands.c

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"

#include <string.h>

#include "logging.h"

static BaseType_t trace_cmd(char* write_buf, size_t write_buf_len, const char* cmd);
static const CLI_Command_Definition_t TRACE_CMD =
{
    "trace",
    "\ntrace: Tracing commands\n  start\n  stop\n",
    trace_cmd,
    1 /* start/stop/restart */
};

void shell_cmds_register(void)
{
    BaseType_t ret;

    ret = FreeRTOS_CLIRegisterCommand(&TRACE_CMD);
    configASSERT(ret == pdPASS);
}

static BaseType_t trace_cmd(char* write_buf, size_t write_buf_len, const char* cmd)
{
    traceResult tr;
    const char* param;
    BaseType_t param_len;
    const char START[] = "start";
    const char STOP[] = "stop";

    configASSERT(write_buf != NULL);

    param = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    configASSERT(param != NULL);

    if(strncmp(param, START, sizeof(START)) == 0)
    {
        if(xTraceIsRecorderEnabled() != 0)
        {
            tr = xTraceDisable();
            configASSERT(tr == TRC_SUCCESS);
            tr = xTraceEnable(TRC_START);
            configASSERT(tr == TRC_SUCCESS);
            strncpy(write_buf, "Tracing restarted\n", write_buf_len);
        }
        else
        {
            tr = xTraceEnable(TRC_START);
            configASSERT(tr == TRC_SUCCESS);
            strncpy(write_buf, "Tracing started\n", write_buf_len);
        }
    }
    else if(strncmp(param, STOP, sizeof(STOP)) == 0)
    {
        strncpy(write_buf, "Tracing stopped\n", write_buf_len);
        tr = xTraceDisable();
        configASSERT(tr == TRC_SUCCESS);
    }
    else
    {
        strncpy(write_buf, "Valid parameters are 'start' and 'stop'\n", write_buf_len);
    }

    // There is no more data to return after this single string
    return pdFALSE;
}
