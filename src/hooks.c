#include "FreeRTOS.h"
#include "task.h"

#include <stdarg.h>
#include <stdio.h>

#include "logging.h"
#include "task_shell.h"
#include "led.h"

void vLoggingPrintf(const char* fmt, ...)
{
    (void) fmt;
    // TODO put this on the INFO or a new DEBUG channel
    //va_list arg;
    //va_start(arg, fmt);
    //vprintf(fmt, arg);
    //va_end(arg);
}

void vApplicationMallocFailedHook(void)
{
    led_on(LED_ALL);
    ERR("vApplicationMallocFailedHook");
    taskDISABLE_INTERRUPTS();
    task_shell_unsafe_printf("vApplicationMallocFailedHook\n");
    while(1)
    {
        portNOP();
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char* name)
{
    (void) task;
    (void) name;

    led_on(LED_ALL);
    ERR("vApplicationStackOverflowHook");
    taskDISABLE_INTERRUPTS();
    task_shell_unsafe_printf("vApplicationStackOverflowHook\n");
    while(1)
    {
        portNOP();
    }
}

void vAssertCalled(const char* file, int line)
{
    traceResult tr;
    TraceStringHandle_t sym;
    const char* err = NULL;
    (void) file;
    (void) line;

    led_on(LED_ALL);
    tr = xTraceStringRegister(file, &sym);
    if(tr == TRC_SUCCESS)
    {
        ERR("vAssertCalled %s:%d", file, line);
    }
    else
    {
        ERR("vAssertCalled");
    }
    taskDISABLE_INTERRUPTS();
    task_shell_unsafe_printf("vAssertCalled %s:%d\n", file, line);

    (void) xTraceErrorGetLast(&err);
    if(err != NULL)
    {
        task_shell_unsafe_printf("xTraceErrorGetLast: %s\n", err);
    }

    while(1)
    {
        portNOP();
    }
}

void assert_failed(uint8_t* file, uint32_t line)
{
    vAssertCalled((const char*) file, line);
}
