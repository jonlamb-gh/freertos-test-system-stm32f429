// More examples:
// https://github.com/FreeRTOS/FreeRTOS/blob/main/FreeRTOS-Plus/Demo/Common/FreeRTOS_Plus_CLI_Demos/Sample-CLI-commands.c

#include "FreeRTOS.h"

#include "FreeRTOS_CLI.h"
#include "FreeRTOS_IP.h"

#include <string.h>

static BaseType_t trace_cmd(char* write_buf, size_t write_buf_len, const char* cmd);
static BaseType_t ip_cmd(char* write_buf, size_t write_buf_len, const char* cmd);

static const CLI_Command_Definition_t TRACE_CMD =
{
    "trace",
    "\ntrace: Tracing commands\n  start\n  stop\n  status\n",
    trace_cmd,
    1 /* start/stop/restart */
};

static const CLI_Command_Definition_t IP_CMD =
{
    "ip",
    "\nip: IP stack commands\n  info\n",
    ip_cmd,
    1 /* info */
};

void shell_cmds_register(void)
{
    BaseType_t ret;

    ret = FreeRTOS_CLIRegisterCommand(&TRACE_CMD);
    configASSERT(ret == pdPASS);

    ret = FreeRTOS_CLIRegisterCommand(&IP_CMD);
    configASSERT(ret == pdPASS);
}

static BaseType_t trace_cmd(char* write_buf, size_t write_buf_len, const char* cmd)
{
    traceResult tr;
    const char* param;
    BaseType_t param_len;
    const char START[] = "start";
    const char STOP[] = "stop";
    const char STATUS[] = "status";

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
    else if(strncmp(param, STATUS, sizeof(STATUS)) == 0)
    {
        if(xTraceIsRecorderEnabled() != 0)
        {
            strncpy(write_buf, "Tracing enabled\n", write_buf_len);
        }
        else
        {
            strncpy(write_buf, "Tracing disabled\n", write_buf_len);
        }
    }
    else
    {
        strncpy(write_buf, "Valid parameters are 'start', 'stop',  and 'status'\n", write_buf_len);
    }

    // There is no more data to return after this single string
    return pdFALSE;
}

static BaseType_t ip_cmd(char* write_buf, size_t write_buf_len, const char* cmd)
{
    const char* param;
    BaseType_t param_len;
    uint32_t ip_addr;
    uint32_t net_mask;
    uint32_t gw_addr;
    uint32_t dns_addr;
    char addr_buffer[16];
    const char INFO[] = "info";

    configASSERT(write_buf != NULL);

    param = FreeRTOS_CLIGetParameter(cmd, 1, &param_len);
    configASSERT(param != NULL);

    if(strncmp(param, INFO, sizeof(INFO)) == 0)
    {
        FreeRTOS_GetAddressConfiguration(&ip_addr, &net_mask, &gw_addr, &dns_addr);
        FreeRTOS_inet_ntoa(ip_addr, addr_buffer);
        snprintf(write_buf, write_buf_len, "IP address: %s\n", addr_buffer);
    }
    else
    {
        strncpy(write_buf, "Valid parameters are 'info'\n", write_buf_len);
    }

    // There is no more data to return after this single string
    return pdFALSE;
}
