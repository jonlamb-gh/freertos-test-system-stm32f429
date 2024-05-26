#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

#include "logging.h"
#include "task_stats.h"

#define TASK_PRIO (tskIDLE_PRIORITY + 1)
#define TASK_STACK_SIZE (2 * configMINIMAL_STACK_SIZE)

#define STATS_INTERVAL pdMS_TO_TICKS(500)
#define MAX_TASKS TRC_CFG_STACK_MONITOR_MAX_TASKS

typedef struct
{
    TaskHandle_t task;
    TraceStringHandle_t sym;
    unsigned long last_runtime_counter;
} task_state_s;

static void stats_task(void* params);
static void log_task_stats(const TaskStatus_t* status, const task_state_s* state, unsigned long total_runtime);
static task_state_s* task_state(const TaskStatus_t* s);

static TraceStringHandle_t g_stats_ch = NULL;
static task_state_s g_task_state[MAX_TASKS] = {0};
static TaskStatus_t g_task_stats[MAX_TASKS] = {0};
static unsigned long g_last_total_runtime = 0;

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

        for(i = 0; i < num_tasks; i += 1)
        {
            task_state_s* state = task_state(&g_task_stats[i]);

            if((total_runtime / 100) != 0)
            {
                log_task_stats(&g_task_stats[i], state, total_runtime);
            }

            state->last_runtime_counter = g_task_stats[i].ulRunTimeCounter;
        }

        g_last_total_runtime = total_runtime;
    }
}

static void log_task_stats(const TaskStatus_t* status, const task_state_s* state, unsigned long total_runtime)
{
    traceResult tr;

    tr = xTracePrintF(
            g_stats_ch,
            "%s %u %u %u",
            state->sym,
            status->usStackHighWaterMark,
            status->ulRunTimeCounter - state->last_runtime_counter,
            total_runtime - g_last_total_runtime);
    configASSERT(tr == TRC_SUCCESS);
}

static task_state_s* task_state(const TaskStatus_t* s)
{
    int i;
    traceResult tr;
    task_state_s* state = NULL;

    // Check if already registered
    for(i = 0; i < MAX_TASKS; i += 1)
    {
        if(g_task_state[i].task == s->xHandle)
        {
            state = &g_task_state[i];
            break;
        }
    }

    // Allocate a new entry
    if(state == NULL)
    {
        for(i = 0; i < MAX_TASKS; i += 1)
        {
            if(g_task_state[i].task == NULL)
            {
                configASSERT(g_task_state[i].sym == NULL);
                tr = xTraceStringRegister(s->pcTaskName, &g_task_state[i].sym);
                configASSERT(tr == TRC_SUCCESS);
                g_task_state[i].task = s->xHandle;
                state = &g_task_state[i];
                break;
            }
        }
    }

    configASSERT(state != NULL);

    return state;
}
