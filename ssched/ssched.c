/**********************************************************
 * 
 *  ssched.c
 * 
 * 
 *  DESCRIPTION:
 *      Simple task scheduler
 *
 */

#include "sched.h"
#include "generic.h"
#include "uart.h"
#include "peripherals/timer.h"
#include "printf.h"
#include "debug.h"

/**
 * $config: SSCHED_TSK_MAX. Maximum number of tasks allowed. This includes
 * kernel tasks which will vary depending on system configuration.
 * 
 */
#ifndef SSCHED_TSK_MAX
#define SSCHED_TSK_MAX  20
    #warning Configuration SCHED_TSK_MAX not set, using default value of 20
#endif

/**
 * $config: SSCHED_SCHED_TICK_US. Microseconds per scheduler "tick",
 * i.e., how often the scheduler will run.
 * 
 */
#ifdef SSCHED_SCHED_TICK_US
#define SSCHED_SCHED_TICK_US 1000
    #warning Configuration SSCHED_SCHED_TICK_US not set, using default value of 1000uS.
#endif

/* Types */

typedef struct
    {
    boolean           cnfgd;    /* Task is configured in the system */
    sched_usr_tsk_t   usr_tsk;  /* User task ptr */
    } task_cb_t;

/**
 * Control variables:
 * 
 *      system_task_list
 * 
 *          List of all tasks that are currently active. Active means
 *          the task is being scheduled. Tasks that are asleep, waiting
 *          for a lock, etc. are NOT active.
 * 
 *      is_sched_running
 * 
 *          Whether or not the scheduler is currently running. The
 *          scheduler can be turned off if, for example, we need
 *          to process time sensitive instructions.
 *  
 *          This kind of thing is typically reserved for kernel tasks, 
 *          i.e., user tasks shouldn't be able to stop the scheduler.
 *
 */
static task_cb_t system_task_list[ SSCHED_TSK_MAX ];
static boolean is_sched_running;
static timer_id_t8 sched_timer_id;
static uint64_t system_tick;
static uint32_t task_id_count;
static uint8_t insert_task_index;

/* Forward declares */
static void sched_task(void);
static void kick_off_sched(void);
static void register_new_task(sched_usr_tsk_t *task);

/**********************************************************
 * 
 *  sched_init()
 * 
 * 
 *  DESCRIPTION:
 *      Initialize the scheduler
 *
 */

void sched_init(sched_usr_tsk_t *tasks, uint32_t num_tasks)
{
    /* local vars */
    uint8_t i;

    /* initialize module variables */
    is_sched_running = FALSE;
    task_id_count = 0;
    system_tick = 0;
    insert_task_index = 0;

    /* make sure uart is initialized so we can debug print */
    if(FALSE == uart_is_init())
    {
        uart_init();
    }

    /* input validation */
    if( NULL == tasks )
    {
        return;
    }

    /* Set up initial task list if provided one */
    for(i = 0; i < num_tasks; i++ )
    {
        register_new_task(&tasks[i]);
    }

    kick_off_sched();

    if(FALSE == is_sched_running)
    {
        printf("Failed to initialize the scheduler\n");
    }
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

static boolean register_new_task(sched_usr_tsk_t *task)
{
        /* Prevent erroneous roll over checks. The simple
        scheduler does not support re-use of task ids. if
        a task is removed, it will just get a new id. IDs
        can exceed the max number of configurable tasks
        SSCHED_TSK_MAX.
        */
        if(system_task_list[task_id_count].cnfgd == TRUE)
        {
            #ifdef SSCHED_SHOW_DEBUG_MSGS
                printf("Task id %d is already registered!", task_id_count);
            #endif

            return FALSE;
        }

        system_task_list[task_id_count].cnfgd = TRUE;
        system_task_list[task_id_count].usr_tsk = *task;
        task->id = task_id_count;

        task_id_count++;

        #ifdef SSCHED_SHOW_DEBUG_DATA
            printf("\nSuccessfully configured task %d\n", task->id);
        #endif

        return TRUE;
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
 *      This function is called 
 *
 */

static void sched_task(void)
{
    if(!is_sched_running)
    {
        return;
    }


}

/**********************************************************
 * 
 *  kick_off_sched()
 * 
 * 
 *  DESCRIPTION:
 *      Start the scheduler
 *
 */

static void kick_off_sched(void)
{
    /* allocate a system timer */
    if( TIMER_ERR_NONE != timer_alloc(&sched_timer_id, sched_task, SSCHED_SCHED_TICK_US))
    {
        is_sched_running = FALSE;
        return;
    }

    is_sched_running = TRUE;
}
