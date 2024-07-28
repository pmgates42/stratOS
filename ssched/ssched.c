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

#include "sched.h"
#include "generic.h"
#include "uart.h"
#include "peripherals/timer.h"
#include "printf.h"
#include "debug.h"

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

typedef struct task_cb_t_struc
    {
    boolean                  alive;        /* Task is not killed */
    boolean                  active        /* Task is actively running on the scheduler */;
    boolean                  scheduled;    /* Task has been scheduled */
    sched_usr_tsk_t          usr_tsk;      /* User task ptr */
    uint64_t                 active_tick;  /* Tick when task became active */
    struct task_cb_t_struc * next_task;    /* Next task to run */
    } task_cb_t;

typedef uint8_t scheduler_state_t;
enum
{
    IDLE,               /* Scheduler is idle */
    TASK_OVERRUN,       /* Hnadle a task overrun */
    EXECUTE_TASK,       /* Execute the task head */
    QUEUE_TASKS,        /* Queue up tasks */
};

/**********************************************************
 * Control variables:
 *
 *      system_task_list
 *
 *          TODO this list is serving two purposes at the moment. It
 *          TODO serves as a list of all tasks that are registered
 *          TODO and also as the active task list. Eventually the idea
 *          TODO is to have multiple lists that all point to a main
 *          TODO list, e.g., an active list, a waiting list, etc.
 *
 *          List of all tasks that are currently active. Active means
 *          the task is being scheduled. Tasks that are asleep, waiting
 *          for a lock, etc. are NOT active.
 *
 *      is_sched_running
 *
 *          Whether or not the scheduler is currently running. The
 *          scheduler can be turned off if, for example, we need
 *          need to do time critical execution in the kernel.
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
 */

static int sched_init_key;
static task_cb_t system_task_list[ SSCHED_TSK_MAX ];
static boolean is_sched_running;
static timer_id_t8 sched_timer_id;
static uint64_t system_tick;
static sched_task_id_t task_id_count;
static scheduler_state_t scheduler_state;
static task_cb_t * task_head;

/* Forward declares */

static void sched_task(void);
static boolean register_new_task(sched_usr_tsk_t *task);
static boolean find_available_task_index(uint8_t *index);
static void call_task_proc(task_cb_t * task);
static void queue_tasks(void);

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
    /* Local vars */
    uint8_t i;

    /* Ensure scheduler was initialized */
    if(sched_init != SCHED_INIT_KEY)
    {
        is_sched_running = FALSE;
        return;
        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nScheduler was not initialized!");
        #endif
    }

    is_sched_running = TRUE;

    while(TRUE)
    {
        /* Scheduler state machine */
        switch(scheduler_state)
        {
            /* Idle state, don't do anything */
            case IDLE:
                break;

            /* Handle task overruns */
            case TASK_OVERRUN:
                scheduler_state = IDLE;
                break;

            /* Execute a queued task */
            case EXECUTE_TASK:
                call_task_proc(task_head);
                scheduler_state = IDLE;
                break;

            /* Perform task queuing */
            case QUEUE_TASKS:
                queue_tasks();
                scheduler_state = IDLE;
                break;

            /* Invalid state. Log a debug message once */
            default:
            #ifdef SSCHED_SHOW_DEBUG_DATA
                static boolean invalid_state_message_shown = FALSE;
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
    scheduler_state = IDLE;
    task_head = NULL;
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
    if( TIMER_ERR_NONE != timer_alloc(&sched_timer_id, sched_task, SSCHED_SCHED_TICK_US))
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
    uint8_t index;

    if(NULL == task)
    {
        return FALSE;
    }

    if(FALSE == find_available_task_index(&index))
    {
        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nFailed to register task!");
        #endif
        return FALSE;
    }

    /* Insert the new task into the active scheduler list.
     * There is a posibility that we are overwriting an
     * inactive task here. The assumption is that this task
     * has either been killed or is on another list waiting
     * to be put back on the scheduler.
     */
    system_task_list[index].alive = TRUE;
    system_task_list[index].usr_tsk = *task;
    task->id = task_id_count;

    task_id_count++;

    return TRUE;
}

static boolean alive_task(sched_usr_tsk_t * task)
{

}

/**********************************************************
 *
 *  find_available_task_index()
 *
 *
 *  DESCRIPTION:
 *      Find the first index in the system task list where
 *      a task is NOT active.
 *
 *      Returns true if an available index was found and
 *      false if one cannot be found.
 *
 */

static boolean find_available_task_index(uint8_t *index)
{
    uint8_t i;

    /* Input validation */
    if(NULL == index)
    {
        return FALSE;
    }

    for(i = 0; i < SSCHED_TSK_MAX; i++)
    {
        if( FALSE == system_task_list[i].active)
        {
            *index = i;
            return TRUE;
        }
    }

    return FALSE;
}

/**********************************************************
 *
 *  sched_task()
 *
 *
 *  DESCRIPTION:
 *      Main scheduler system task. Handles scheduling of
 *      all other user and kernel tasks.
 *
 *  NOTES:
 *      This function is called every tick of the registered
 *      scheduler timer, i.e., every SSCHED_SCHED_TICK_US
 *      microseconds.
 *
 */

static void sched_task(void)
{
    #define DETECT_OVERRUN(tsk) ( tsk->usr_tsk.period_ms < ( ( system_tick - tsk->active_tick ) * MS_PER_TICKS ) )

    /* Local vars */
    uint64_t task_active_ms;

    /* Check for system tick roll over */
    if((system_tick + 1) == 0)
        // TODO handle system tick roll over
        printf("System tick roll over detected");

    system_tick++;

    /* Scheduler is not running or no task has been scheduled */
    if(!is_sched_running || task_head == NULL)
    {
        return;
    }

    /* Allow active task to continue to run. Check for task 
       overruns */
    if(TRUE == task_head->active)
    {
        if(DETECT_OVERRUN(task_head))
        {
        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nTask %d did an overrun");
        #endif
        scheduler_state = TASK_OVERRUN;
        }
        return;
    }

    /* Queue tasks as needed */
    scheduler_state = QUEUE_TASKS;
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
    if( task != NULL  && task->usr_tsk.task_func )
    {
        task->active_tick = system_tick;
        task->usr_tsk.task_func();
    }

    task->active = FALSE;

    /* Task can only exist once in the task scheduling chain */
    task->scheduled = FALSE;

}

/**********************************************************
 *
 *  call_task_proc()
 *
 *
 *  DESCRIPTION:
 *      Queue tasks to be ran on the scheduler. This is where
 *      the scheduler sausage is made.
 *
 *  NOTES:
 *      Function assumes that the task peroid is less than the
 *      max value of a signed integer.
 *
 *      TODO handle system_tick roll over
 *
 */

static void queue_tasks(void)
{
    uint8_t     i;
    uint32_t    period_ms;         /* Task period in ms */
    uint32_t    last_ran_ms;       /* Last run of task in ms */
    sint32_t    next_exe_ms;       /* Next time of execution */
    sint32_t    sml_next_exe_ms;   /* Smallest time of next execution */
    sint32_t    next_tsk_idx;      /* Index of next task to run */
    task_cb_t * task_ptr;          /* Pointer to task cb */

    sml_next_exe_ms = ~( 0 );

    /* Find the active task that will need to be ran the
    the soonest */
    for(i = 0; i < SSCHED_TSK_MAX; i++)
    {
        if( system_task_list[i].scheduled == TRUE || system_task_list->alive == FALSE )
            continue;

        period_ms = system_task_list[i].usr_tsk.period_ms;
        last_ran_ms = system_task_list[i].active_tick * MS_PER_TICKS;

        next_exe_ms = ( sint32_t )period_ms - ( sint32_t )( system_tick - last_ran_ms );

        if( next_exe_ms < sml_next_exe_ms)
        {
            sml_next_exe_ms = next_exe_ms;
            next_tsk_idx = i;
        }
    }

    /* Determine if the next task is ready to be ran */
    if( sml_next_exe_ms <= 0 )
    {
        if( task_head != NULL )
        {
            /* Find the end of the task list and insert task */
            task_ptr = task_head;
            while( task_ptr->next_task != NULL )
            {
                task_ptr = task_head->next_task;
            }

            task_ptr->next_task = &system_task_list[next_tsk_idx];
            task_ptr->next_task->scheduled = TRUE;
        }
        /* task_head is NULL (less common case) */
        else
        {
            task_head = &system_task_list[next_tsk_idx];
            task_head->scheduled = TRUE;
        }
    }
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
        if(system_task_list[index].usr_tsk.id == task_id)
        {
            system_task_list[index].active = FALSE;
            system_task_list[index].alive = FALSE;

            return TRUE;
        }
   }

    return FALSE;
}
