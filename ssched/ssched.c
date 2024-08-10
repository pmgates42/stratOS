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
#define SSCHED_SCHED_TICK_US 1000
    #warning Configuration SSCHED_SCHED_TICK_US not set, using default value of 1000uS.
#endif

#define US_PER_MS 1000
#define MS_PER_TICKS ( SSCHED_SCHED_TICK_US / US_PER_MS )
#define SCHED_INIT_KEY 0x78DEF087

/* Types */

/**********************************************************
 * task_cb_t_struc:
 *
 *      Internal task control block type
 * 
 *      alive
 * 
 *          A task is considered alive if it has not been
 *          killed, i.e., if sched_kill_task() has not been
 *          called on it.
 *
 *      active
 * 
 *          A task is considered active if it is a canidate
 *          for scheduling. Some examples of when a task is
 *          not active would be if it is waiting on a lock
 *          or some other condition before it can be ran.
 * 
 *      scheduled
 *
 *          Task is actively running.
 * 
 *      usr_tsk
 * 
 *          User task definition provided durring task
 *          registration.
 *
 *      active_tick
 *
 *          Tick when the task became active.
 *
 *      cycle_end_tick
 *
 *          Tick when the task finished executing.
 *
 *      next_task
 *
 *          Next task to run on the scheduler.
 *          
 */

typedef struct task_cb_t_struc
    {
    boolean                  alive;
    boolean                  active;
    boolean                  scheduled;
    sched_usr_tsk_t        * usr_tsk;
    uint64_t                 active_tick;
    uint64_t                 cycle_end_tick;
    struct task_cb_t_struc * next_task;
    } task_cb_t;

typedef uint8_t scheduler_state_t;
enum
{
    INIT,               /* Scheduler is initialized */
    IDLE,               /* Scheduler is idle */
    TASK_OVERRUN,       /* Hnadle a task overrun */
    EXECUTE_TASK,       /* Execute the task head */
    SCHEDULE_TASKS,     /* Queue up tasks */
};

/**********************************************************
 * Control variables:
 *
 *      system_task_list
 *
 *          List of all registered tasks.
 *
 *      is_sched_running
 *
 *          Whether or not the scheduler is currently running. The
 *          scheduler can be turned off if, for example, we need
 *          need to do time critical execution in the kernel.
 * 
 *      scheduler_is_booting
 *
 *          True if scheduler is booting up for the first time.
 * 
 *      sched_timer_id
 *
 *          ID for system timer instance that scheduler runs off of.
 *
 *      system_tick
 *
 *          Current system tick. Ticks are incremented every scheduler
 *          task iteration regardless of whether or not the scheduler
 *          is running.
 *
 *      task_id_count
 *
 *          Count of task IDs for all past and present registered tasks.
 *          There is no roll over support for task IDs at this time,
 *          but this is a 32-bit integer.
 *
 *      scheduler_state
 *
 *          State that the scheduler is in.
 *
 *      registered_tasks
 *
 *          Count of registered tasks
 */

static int sched_init_key;
static task_cb_t system_task_list[ SSCHED_TSK_MAX_REGISTERED ];
static boolean is_sched_running;
static boolean scheduler_is_booting;
static timer_id_t8 sched_timer_id;
static uint64_t system_tick;
static uint32_t task_id_count;
static scheduler_state_t scheduler_state;
static task_cb_t * task_head;
static uint32_t registered_tasks;

/* Forward declares */

static void schedule_isr(void);
static boolean register_new_task(sched_usr_tsk_t *task);
static void call_task_proc(task_cb_t * task);

/**********************************************************
 *
 *  sched_main()
 *
 *  DESCRIPTION:
 *      Main scheduling function. Control is passed to this
 *      function after kernel is fully booted.
 *
 *      This function is used to process the output from
 *      the scheduler which helps to minimize execution
 *      time during the system timer interrupt.
 *
 */

void sched_main(void)
{
#ifdef SSCHED_SHOW_DEBUG_DATA
    static boolean invalid_state_message_shown = FALSE;
#endif

    /* Ensure scheduler was initialized */
    if(sched_init_key != SCHED_INIT_KEY)
    {
        return;
        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nScheduler was not initialized before control was passed to it! Scheduler will not run. Call sched_init() to fix this.");
        #endif
    }

    is_sched_running = FALSE;

    while(TRUE)
    {
        /* Scheduler state machine */
        switch(scheduler_state)
        {
            case INIT:
                break;

            /* Idle state */
            case IDLE:
                break;

            /* Handle task overruns (unused) */
            case TASK_OVERRUN:
                break;

            /* Execute a queued task */
            case EXECUTE_TASK:
                call_task_proc(task_head);
                scheduler_state = IDLE;
                break;

            /* Invalid state. Log a debug message once */
            default:
            #ifdef SSCHED_SHOW_DEBUG_DATA
                if(invalid_state_message_shown == FALSE)
                {
                    printf("\nInvalid scheduler state!");
                    invalid_state_message_shown = TRUE;
                }
            #endif
                break;
        }
    }
}


/**********************************************************
 *
 *  sched_init()
 *
 *
 *  DESCRIPTION:
 *      Contract function. Initialize the scheduler. Called
 *      during kernel initialization.
 *
 */

sched_err_t sched_init(sched_usr_tsk_t *tasks, uint32_t num_tasks)
{
    /* local vars */
    uint8_t i;

    /* initialize the control variables */
    sched_init_key = SCHED_INIT_KEY;
    is_sched_running = FALSE;
    task_id_count = 0;
    system_tick = 0;
    registered_tasks = 0;
    task_head = NULL;
    scheduler_is_booting = TRUE;
    clr_mem(system_task_list, sizeof(system_task_list));

    /* make sure uart is initialized so we can debug print */
    if(FALSE == uart_is_init())
    {
        uart_init();
    }

    /* input validation */
    if( num_tasks > 0 && NULL == tasks )
    {
        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nInitializing scheduler with zero tasks");
        #endif

        return SCHED_ERR_PARAM;
    }

    /* Set up initial task list if provided one */
    for(i = 0; i < num_tasks; i++ )
    {
        register_new_task(&tasks[i]);
    }

    /* allocate a system timer */
    if( TIMER_ERR_NONE != timer_alloc(&sched_timer_id, schedule_isr, SSCHED_SCHED_TICK_US))
    {
        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nFailed to allocate a system timer. Cannot run scheduler.");
        #endif
        return SCHED_ERR_INVLD_STATE;
    }

    return SCHED_ERR_NO_ERR;
}


/**********************************************************
 *
 *  sched_register_task()
 *
 *
 *  DESCRIPTION:
 *      Contracted function to register a task.
 *
 */

sched_err_t sched_register_task(sched_usr_tsk_t * task)
{
    if(register_new_task(task))
    {
        return SCHED_ERR_FAILED_REG;
    }
    return SCHED_ERR_NO_ERR;
}

/**********************************************************
 *
 *  register_new_task()
 *
 *
 *  DESCRIPTION:
 *      Register a new task with the system.
 *
 *  NOTES:
 *      TODO
 *
 *      At the moment there is no distinction between
 *      user tasks and kernel tasks.
 *
 */

static boolean register_new_task(sched_usr_tsk_t * task)
{
    if(NULL == task || task_id_count > SSCHED_TSK_MAX_REGISTERED || task->task_func == NULL)
    {
    #ifdef SSCHED_SHOW_DEBUG_DATA
        printf("\nFailed to register task! task_null=%d, max_tasks=%d, task_func_null=%d", (NULL == task), (task_id_count < SSCHED_TSK_MAX_REGISTERED), (task->task_func == NULL) );
    #endif
        return FALSE;
    }

    system_task_list[task_id_count].usr_tsk = task;
    system_task_list[task_id_count].alive = TRUE;

    /* gaurd against init failure */
    system_task_list[task_id_count].active = FALSE;
    system_task_list[task_id_count].active_tick = 0;
    system_task_list[task_id_count].cycle_end_tick = 0;
    system_task_list[task_id_count].next_task = NULL;
    system_task_list[task_id_count].scheduled = FALSE;

    /*
     * At some point in the future the task id should be
     * calculated using a perfect hashing function.
     * 
     * https://github.com/pmgates42/stratOS/issues/1
     */
    task->id = task_id_count;

    task_id_count++;
    registered_tasks++;

    return TRUE;
}

/**********************************************************
 *
 *  schedule_isr()
 *
 *
 *  DESCRIPTION:
 *      Scheduler ISR.
 *
 *  NOTES:
 *      This function is called every tick of the registered
 *      scheduler timer, i.e., every SSCHED_SCHED_TICK_US
 *      microseconds.
 *
 */

static void schedule_isr(void)
{
    //TODO put this in an inline function
    #define setup_task_to_run(task_ptr)                 \
        {                                               \
            task_head = task_ptr;                       \
            task_head->active_tick = system_tick;       \
            task_head->scheduled = TRUE;                \
                                                        \
            scheduler_state = EXECUTE_TASK;             \
        }                                               \
    
    uint32_t i;


    /* Check for system tick roll over */
    if((system_tick + 1) == 0)
        // TODO handle system tick roll over
        printf("System tick roll over detected");

    system_tick++;

    /* Scheduler is booted and current task has finished running */
    if( (!scheduler_is_booting) && task_head != NULL && task_head->scheduled == FALSE )
    {
        /* See if any tasks are ready to run */
        for(i = 0; i < registered_tasks; i++)
        {
            if( system_task_list[i].usr_tsk != NULL
            &&( system_tick >= ( system_task_list[i].active_tick + system_task_list[i].usr_tsk->period_ms ) ) )
            {
                setup_task_to_run( &system_task_list[i] );
            }
        }
    }

    /*  Scheduler was just initialized or at least one task
     *  is alive after a period of no tasks being alive.
     *
     *  This should always be the last condition checked in
     *  the scheduling algorithm since it will likely occur
     *  less frequently than other conditions.
     */
    else if( scheduler_is_booting == TRUE )
    {
        /* Find the first alive task and execute it */
        for(i = 0; i < SSCHED_TSK_MAX_REGISTERED; i++)
        {
            if( system_task_list[ i ].alive == TRUE )
                {
                    setup_task_to_run( &system_task_list[i] );
                    scheduler_is_booting = FALSE;
                    break;             
                }
        }
    }

    /* Handle time based events */

    switch(scheduler_state)
    {
        /* EXECUTING A TASK */
        case EXECUTE_TASK:
        {
        #define DETECT_OVERRUN(tsk) ( ( ( system_tick - tsk->active_tick ) * MS_PER_TICKS ) > tsk->usr_tsk->period_ms )

            /* check for task overrun */
            if( DETECT_OVERRUN(task_head) )
            {
            scheduler_state = TASK_OVERRUN;

            //TODO $task_stats: collect overrun data here

        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nTask overrun has occured on task with id=%d. Consider lengthening period_ms on task registration.", task_head->usr_tsk->id);
        #endif
            }
        #undef DETECT_OVERRUN
        }
        break;

        /* EXECUTING TASK HAS OVERRUN CYCLE */
        case TASK_OVERRUN:
        {
            // TODO wait a little longer and see if task finishes executing
            // TODO if crosses threshold then kill this task

        }
        break;
    }
}

/**********************************************************
 *
 *  call_task_proc()
 *
 *
 *  DESCRIPTION:
 *      Call the task procedure and update control vars.
 *
 */

static void call_task_proc(task_cb_t * task)
{
    if( task != NULL && task->usr_tsk->task_func )
    { 
    #ifdef SSCHED_SHOW_DEBUG_DATA
        if(task->scheduled == FALSE)
            printf("Invalid state! Only scheduled tasks should be executed!");
    #endif

        task->usr_tsk->task_func();//TODO pass in flags
    }
    /* Theoritially should never execute */
    else
    {
    #ifdef SSCHED_SHOW_DEBUG_DATA
        printf("Tried to execute NULL task or task with missing task_func!");
    #endif
    }

    /* Task has finished running */
    task->scheduled = FALSE;
    task->cycle_end_tick = system_tick;

    #ifdef SSCHED_LOG_TASK_STATS
        ssched_log_insert_task_cycle_stat_entry();// TODO doesn't do anything yet
    #endif
}


/**********************************************************
 *
 *  sched_kill_task()
 *
 *
 *  DESCRIPTION:
 *      Contracted scheduler function.
 *
 *      Kill a task and update any control list it lives
 *      in. Return true if we can find the task in a list
 *      and false otherwise.
 *
 */

sched_err_t sched_kill_task(sched_task_id_t task_id)
{
    uint8_t index;

    /* Find the task in the active system task list
    and update it.
    */
   for(index = 0; index < SSCHED_TSK_MAX; index++)
   {
        if(system_task_list[index].usr_tsk->id == task_id)
        {
            system_task_list[index].active = FALSE;
            system_task_list[index].alive = FALSE;

            return SCHED_ERR_NO_ERR;
        }
   }

    return SCHED_ERR_FAILED_UPDATE;
}

/**********************************************************
 *
 *  sched_activate_task()
 *
 *
 *  DESCRIPTION:
 *      Contracted scheduler function.
 *
 */

sched_err_t sched_activate_task(sched_task_id_t task_id)
{
    (void)task_id;
    return SCHED_ERR_FAILED_UPDATE;
}