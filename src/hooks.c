#include "FreeRTOS.h"
#include "task.h"

#include <stdarg.h>
#include <stdio.h>

#include "logging.h"
#include "led.h"

void vLoggingPrintf(const char* fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    vprintf(fmt, arg);
    va_end(arg);
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    led_on(LED_ALL);
    ERR("vApplicationMallocFailedHook");
    while(1)
    {
        portNOP();
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char* name)
{
    (void) task;
    (void) name;

    taskDISABLE_INTERRUPTS();
    led_on(LED_ALL);
    ERR("vApplicationStackOverflowHook");
    while(1)
    {
        portNOP();
    }
}

void vAssertCalled(const char* file, int line)
{
    //const char* err = NULL;
    (void) file;
    (void) line;

    taskDISABLE_INTERRUPTS();
    led_on(LED_ALL);
    ERR("vAssertCalled");

    /*
    (void) xTraceErrorGetLast(&err);
    if(err != NULL)
    {
        // TODO use ERR, intern strings
        //printf("xTraceErrorGetLast: %s\n", err);
    }
    */

    while(1)
    {
        portNOP();
    }
}

void assert_failed(uint8_t* file, uint32_t line)
{
    vAssertCalled((const char*) file, line);
}
