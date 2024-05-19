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
#ifdef SSCHED_SCHED_TICK_US
#define SSCHED_SCHED_TICK_US 1000
    #warning Configuration SSCHED_SCHED_TICK_US not set, using default value of 1000uS.
#endif

/* Types */

typedef struct
    {
    boolean           alive;    /* Task is not killed */
    boolean           active    /* Task is actively running on the scheduler */;
    sched_usr_tsk_t   usr_tsk;  /* User task ptr */
    } task_cb_t;

typedef uint8_t scheduler_state_t;
enum
{
    IDLE,
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

static task_cb_t system_task_list[ SSCHED_TSK_MAX ];
static boolean is_sched_running;
static timer_id_t8 sched_timer_id;
static uint64_t system_tick;
static sched_task_id_t task_id_count;
static uint8_t insert_task_index;
static scheduler_state_t scheduler_state;

/* Forward declares */

static void sched_task(void);
static void kick_off_sched(void);
static void register_new_task(sched_usr_tsk_t *task);
static boolean find_available_task_index(uint8_t *index);

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
    is_sched_running = TRUE;

    while(TRUE)
    {
        switch(scheduler_state)
        {
            /* Idle state, don't do anything */
            case IDLE:
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
    is_sched_running = FALSE;
    task_id_count = 0;
    system_tick = 0;
    scheduler_state = IDLE;
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
            printf("\nInvalid parameters passed to scheduler");
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

sched_err_t sched_register_task(sched_usr_tsk_t task)
{
    if(register_new_task(task))
    {
        return SCHED_ERR_FAILED_REG;
    }
    return SCHED_ERR_NO_ERR:
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
 *      user tasks and a kernel task.
 *
 */

static boolean register_new_task(sched_usr_tsk_t *task)
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
    system_task_list[index].active = TRUE;
    system_task_list[index].usr_tsk = *task;
    task->id = task_id_count;

    task_id_count++;

    return TRUE;
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
    uint8_t i;

    /* Check for system tick roll over */
    if((system_tick + 1) == 0)
        // TODO handle system tick roll over
        printf("System tick roll over detected");

    system_tick++;

    if(!is_sched_running)
    {
        return;
    }

    /* determine if we should execute a new task */
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
static void sched_kill_task(sched_task_id_t task_id)
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
