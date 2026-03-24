/**********************************************************
 *
 *  ssched.c
 *
 *
 *  DESCRIPTION:
 *      Simple task scheduler
 *
 *  NOTES:
 *
 *      Run with SSCHED_SHOW_DEBUG_DATA defined to show debug
 *      messages.
 *
 */
#ifdef EMBEDDED_BUILD
#include "printf.h"
#else
#include <stdio.h>
#endif

#include "sched.h"
#include "generic.h"
#include "uart.h"
#include "peripherals/timer.h"
#include "debug.h"
#include "ssched.h"

/**
 * $config: SSCHED_TSK_MAX. Maximum number of tasks allowed to be ran on the
 * scheduler at once. This includes kernel tasks which will vary depending on
 * system configuration.
 *
 */
#ifndef SSCHED_TSK_MAX
#define SSCHED_TSK_MAX  20
    #warning Configuration SCHED_TSK_MAX not set, using default value of 20
#endif

/**
 * max amount of tasks that can be registered in the system task list.
 * Arbitrarily set to 2X the allowed active tasks, may change later.
 *
 * It may also be useful to have a configuration for this for boards
 * that are more memory conscious.
 *
 */
#define SSCHED_TSK_MAX_REGISTERED ( SSCHED_TSK_MAX * 2 )

/**
 * $config: SSCHED_SCHED_TICK_US. Microseconds per scheduler "tick",
 * i.e., how often the scheduler will run. It is probably best to
 * set this to a value that is in line with the same order of
 * magnitude as the most common task periods, e.g., if tasks are
 * apx. 5Hz, then 1ms would be more appropriate than 1uS.
 * Also note, the tick percision could vary depending on the CPU
 * and underlying timer hardware. It is important to understand
 * the limitations of your system before choosing a tick value,
 * but generally, a value of 1ms or larger is recommended.
 *
 */
#ifndef SSCHED_SCHED_TICK_US
#define SSCHED_SCHED_TICK_US 1000*10 /* 10 ms */
    #warning Configuration SSCHED_SCHED_TICK_US not set, using default value of 1000uS.
#endif

#define US_PER_MS 1000
#define MS_PER_TICKS ( SSCHED_SCHED_TICK_US / US_PER_MS )
#define PERIOD_MS_TO_US(period_ms) ( (uint64_t)(period_ms) * (uint64_t)US_PER_MS )
#define SCHED_INIT_KEY 0x78DEF087

#if (SSCHED_SCHED_TICK_US < US_PER_MS) || ((SSCHED_SCHED_TICK_US % US_PER_MS) != 0)
    #error SSCHED_SCHED_TICK_US must be an integer number of milliseconds (>= 1000 us)
#endif

/* Types */

typedef struct task_cb_t_struc
{
    boolean           alive;
    boolean           active;
    boolean           scheduled;
    sched_usr_tsk_t * usr_tsk;
    uint64_t          active_tick;
    uint64_t          cycle_end_tick;
    uint32_t          period_ticks;
    uint64_t          next_release_tick;
} task_cb_t;

typedef uint8_t scheduler_state_t;
enum
{
    INIT,
    IDLE,
    TASK_OVERRUN,
    EXECUTE_TASK,
};

static int sched_init_key;
static task_cb_t system_task_list[ SSCHED_TSK_MAX_REGISTERED ];
static boolean is_sched_running;
static timer_id_t8 sched_timer_id;
static volatile uint64_t system_tick;
static uint32_t task_id_count;
static volatile scheduler_state_t scheduler_state;
static task_cb_t * task_head;
static uint32_t registered_tasks;
static uint32_t rr_next_index;

/* Forward declares */

static void schedule_isr(void);
static boolean register_new_task(sched_usr_tsk_t *task);
static void run_task(task_cb_t * task);
static task_cb_t * find_next_ready_task(uint64_t now_tick);
static task_cb_t * find_task_by_id(sched_task_id_t task_id);
#ifdef SSCHED_SHOW_DEBUG_DATA
static void debug_print_scheduler_state(void);
#endif

static task_cb_t * find_task_by_id(sched_task_id_t task_id)
{
    uint32_t i;

    for(i = 0; i < registered_tasks; i++)
    {
        if(system_task_list[i].usr_tsk != NULL && system_task_list[i].usr_tsk->id == task_id)
        {
            return &system_task_list[i];
        }
    }

    return NULL;
}

static task_cb_t * find_next_ready_task(uint64_t now_tick)
{
    uint32_t i;

    if(registered_tasks == 0)
    {
        return NULL;
    }

    for(i = 0; i < registered_tasks; i++)
    {
        uint32_t idx = (rr_next_index + i) % registered_tasks;
        task_cb_t * task = &system_task_list[idx];

        if(task->alive == FALSE || task->active == FALSE || task->usr_tsk == NULL)
        {
            continue;
        }

        if(now_tick >= task->next_release_tick)
        {
            rr_next_index = (idx + 1) % registered_tasks;
            return task;
        }
    }

    return NULL;
}

static void run_task(task_cb_t * task)
{
    uint64_t elapsed_us;
    uint64_t period_us;

    if(task == NULL || task->usr_tsk == NULL || task->usr_tsk->task_func == NULL)
    {
        return;
    }

    task_head = task;
    scheduler_state = EXECUTE_TASK;
    task->scheduled = TRUE;
    task->active_tick = system_tick;

    task->usr_tsk->task_func();

    task->scheduled = FALSE;
    task->cycle_end_tick = system_tick;

    elapsed_us = (task->cycle_end_tick - task->active_tick) * (uint64_t)SSCHED_SCHED_TICK_US;
    period_us = PERIOD_MS_TO_US(task->usr_tsk->period_ms);

    if(elapsed_us > period_us)
    {
        scheduler_state = TASK_OVERRUN;
    #ifdef SSCHED_SHOW_DEBUG_DATA
        printf("\nTask overrun occurred on task id=%u. elapsed_us=%u, period_us=%u",
               (unsigned int)task->usr_tsk->id,
               (unsigned int)elapsed_us,
               (unsigned int)period_us);
    #endif
    }
    else
    {
        scheduler_state = IDLE;
    }

    task->next_release_tick += task->period_ticks;
    while(task->next_release_tick <= system_tick)
    {
        task->next_release_tick += task->period_ticks;
    }
}

void sched_main(void)
{
    uint64_t processed_tick;

    if(sched_init_key != SCHED_INIT_KEY)
    {
    #ifdef SSCHED_SHOW_DEBUG_DATA
        printf("\nScheduler was not initialized before control was passed to it.");
    #endif
        return;
    }

    is_sched_running = TRUE;
    processed_tick = system_tick;

    while(TRUE)
    {
        uint64_t now_tick;
        task_cb_t * ready_task;

        if(processed_tick >= system_tick)
        {
            continue;
        }

        while(processed_tick < system_tick)
        {
            processed_tick++;
            now_tick = processed_tick;

            /* Drain all tasks that are ready at this tick before moving to the next tick. */
            while(TRUE)
            {
                ready_task = find_next_ready_task(now_tick);
                if(ready_task == NULL)
                {
                    scheduler_state = IDLE;
                    break;
                }

                run_task(ready_task);
            }
        }

    #ifdef SSCHED_SHOW_DEBUG_DATA
        debug_print_scheduler_state();
    #endif

    #ifdef SSCHED_LOG_TASK_STATS
        ssched_log_insert_task_cycle_stat_entry();
    #endif
    }
}

sched_err_t sched_init(sched_usr_tsk_t *tasks, uint32_t num_tasks)
{
    uint32_t i;

    sched_init_key = SCHED_INIT_KEY;
    is_sched_running = FALSE;
    task_id_count = 0;
    system_tick = 0;
    registered_tasks = 0;
    task_head = NULL;
    scheduler_state = INIT;
    rr_next_index = 0;
    clr_mem(system_task_list, sizeof(system_task_list));

    if(FALSE == uart_is_init())
    {
        uart_init();
    }

    if(num_tasks > 0 && tasks == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    for(i = 0; i < num_tasks; i++)
    {
        if(FALSE == register_new_task(&tasks[i]))
        {
        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nFailed to register startup task index=%u", (unsigned int)i);
        #endif
            return SCHED_ERR_FAILED_REG;
        }
    }

    if(TIMER_ERR_NONE != timer_alloc(&sched_timer_id, schedule_isr, SSCHED_SCHED_TICK_US))
    {
        return SCHED_ERR_INVLD_STATE;
    }

    scheduler_state = IDLE;
    return SCHED_ERR_NO_ERR;
}

sched_err_t sched_register_task(sched_usr_tsk_t * task)
{
    return register_new_task(task) ? SCHED_ERR_NO_ERR : SCHED_ERR_FAILED_REG;
}

static boolean register_new_task(sched_usr_tsk_t * task)
{
    task_cb_t * cb;

    if(task == NULL || task_id_count >= SSCHED_TSK_MAX_REGISTERED || task->task_func == NULL)
    {
        return FALSE;
    }

    if((task->period_ms == 0) || ((task->period_ms % MS_PER_TICKS) != 0))
    {
    #ifdef SSCHED_SHOW_DEBUG_DATA
        printf("\nFailed to register task! period_ms=%u is invalid for scheduler tick resolution of %u ms.",
               (unsigned int)task->period_ms,
               (unsigned int)MS_PER_TICKS);
    #endif
        return FALSE;
    }

    cb = &system_task_list[task_id_count];
    cb->usr_tsk = task;
    cb->alive = TRUE;
    cb->active = TRUE;
    cb->scheduled = FALSE;
    cb->active_tick = 0;
    cb->cycle_end_tick = 0;
    cb->period_ticks = (uint32_t)(task->period_ms / MS_PER_TICKS);
    cb->next_release_tick = system_tick;

    task->id = task_id_count;
    task_id_count++;
    registered_tasks++;

    if(registered_tasks == 1)
    {
        rr_next_index = 0;
    }

    return TRUE;
}

static void schedule_isr(void)
{
    system_tick++;
}

#ifdef SSCHED_SHOW_DEBUG_DATA
static void debug_print_scheduler_state(void)
{
    debug_printf("\n");
    debug_printf("*****DEBUG SCHEDULER*****");
    debug_printf("scheduler_state=%d", scheduler_state);
    debug_printf(" system_tick=%u", (unsigned int)system_tick);
    debug_printf("\n");
}
#endif

sched_err_t sched_kill_task(sched_task_id_t task_id)
{
    task_cb_t * task = find_task_by_id(task_id);

    if(task == NULL)
    {
        return SCHED_ERR_FAILED_UPDATE;
    }

    task->active = FALSE;
    task->alive = FALSE;
    task->scheduled = FALSE;

    if(task_head == task)
    {
        task_head = NULL;
    }

    return SCHED_ERR_NO_ERR;
}

sched_err_t sched_activate_task(sched_task_id_t task_id)
{
    task_cb_t * task = find_task_by_id(task_id);

    if(task == NULL)
    {
        return SCHED_ERR_FAILED_UPDATE;
    }

    if(task->alive == FALSE)
    {
        return SCHED_ERR_FAILED_UPDATE;
    }

    task->active = TRUE;
    task->next_release_tick = system_tick;

    return SCHED_ERR_NO_ERR;
}