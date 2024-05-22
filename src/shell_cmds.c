#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"

#include <string.h>

#include "logging.h"

static BaseType_t example_cmd(char* write_buf, size_t write_buf_len, const char* cmd);
static const CLI_Command_Definition_t EXAMPLE_CMD =
{
    "example",
    "\r\nexample:\r\n This is an example\r\n",
    example_cmd,
    0
};

static const CLI_Command_Definition_t EXAMPLE2_CMD =
{
    "example2",
    "\r\nexample2:\r\n This is an example\r\n",
    example_cmd,
    0
};

void shell_cmds_register(void)
{
    BaseType_t ret;

    ret = FreeRTOS_CLIRegisterCommand(&EXAMPLE_CMD);
    configASSERT(ret == pdPASS);
    
    ret = FreeRTOS_CLIRegisterCommand(&EXAMPLE2_CMD);
    configASSERT(ret == pdPASS);
}

static BaseType_t example_cmd(char* write_buf, size_t write_buf_len, const char* cmd)
{
    const char MSG[] = "This is output from example cmd.\r\n";
    (void) cmd;

    configASSERT(write_buf != NULL);

    strncpy(write_buf, MSG, write_buf_len);

    // There is no more data to return after this single string
    return pdFALSE;
}
