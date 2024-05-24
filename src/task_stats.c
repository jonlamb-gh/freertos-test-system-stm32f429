#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#include "logging.h"
#include "task_stats.h"

#define TASK_PRIO (tskIDLE_PRIORITY + 1)
#define TASK_STACK_SIZE (2 * configMINIMAL_STACK_SIZE)

#define STATS_INTERVAL pdMS_TO_TICKS(1000)
#define MAX_TASKS TRC_CFG_STACK_MONITOR_MAX_TASKS

typedef struct
{
    TraceStringHandle_t sym;
    const char* name;
} task_name_s;

static void stats_task(void* params);
static void log_task_stats(const TaskStatus_t* s, unsigned long total_runtime);
static TraceStringHandle_t task_name_sym(const char* name);

static TraceStringHandle_t g_stats_ch = NULL;
static task_name_s g_task_names[MAX_TASKS] = {0};
static TaskStatus_t g_task_stats[MAX_TASKS] = {0};

void task_stats_start(void)
{
    BaseType_t ret;

    ret = xTaskCreate(
            stats_task,
            "Stats",
            TASK_STACK_SIZE,
            NULL,
            TASK_PRIO,
            NULL);
    configASSERT(ret == pdPASS);
}

static void stats_task(void* params)
{
    UBaseType_t i;
    traceResult tr;
    TickType_t next_wake;
    UBaseType_t num_tasks;
    unsigned long total_runtime;
    (void) params;

    tr = xTraceStringRegister("stats", &g_stats_ch);
    configASSERT(tr == TRC_SUCCESS);

    next_wake = xTaskGetTickCount();
    while(1)
    {
        const BaseType_t was_delayed = xTaskDelayUntil(&next_wake, STATS_INTERVAL);
        if(was_delayed == pdFALSE)
        {
            WARN("Deadline missed");
        }

        num_tasks = uxTaskGetNumberOfTasks();
        configASSERT(num_tasks <= MAX_TASKS);

        num_tasks = uxTaskGetSystemState(g_task_stats, num_tasks, &total_runtime);
        configASSERT(num_tasks != 0);

        if((total_runtime / 100) != 0)
        {
            for(i = 0; i < num_tasks; i += 1)
            {
                log_task_stats(&g_task_stats[i], total_runtime);
            }
        }
    }
}

static void log_task_stats(const TaskStatus_t* s, unsigned long total_runtime)
{
    traceResult tr;

    tr = xTracePrintF(
            g_stats_ch,
            "%s %u %u %u",
            task_name_sym(s->pcTaskName),
            s->usStackHighWaterMark,
            s->ulRunTimeCounter,
            total_runtime);
    configASSERT(tr == TRC_SUCCESS);
}

static TraceStringHandle_t task_name_sym(const char* name)
{
    int i;
    traceResult tr;
    TraceStringHandle_t sym = NULL;

    // Check if already registered
    for(i = 0; i < MAX_TASKS; i += 1)
    {
        if(g_task_names[i].sym != NULL)
        {
            if(strncmp(g_task_names[i].name, name, configMAX_TASK_NAME_LEN) == 0)
            {
                sym = g_task_names[i].sym;
                break;
            }
        }
    }

    // Allocate a new entry
    if(sym == NULL)
    {
        for(i = 0; i < MAX_TASKS; i += 1)
        {
            if(g_task_names[i].sym == NULL)
            {
                configASSERT(g_task_names[i].name == NULL);
                tr = xTraceStringRegister(name, &sym);
                configASSERT(tr == TRC_SUCCESS);
                g_task_names[i].sym = sym;
                g_task_names[i].name = name;
                break;
            }
        }
    }

    configASSERT(sym != NULL);

    return sym;
}
