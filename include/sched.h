/**********************************************************
 *
 *  sched.h
 *
 *  DESCRIPTION:
 *      Task scheduler interface
 *
 */

#include "generic.h"

typedef uint32_t sched_task_id_t;


/**********************************************************
 *
 *  sched_usr_tsk_t
 *
 *  period_ms
 *
 *      period in milliseconds that the task should run. It
 *      is important to consider the 1. configured scheduler
 *      and 2. other tasks and their periods, otherwise task
 *      may not be able to run at this period (e.g., due to
 *      overruns, missed duty cycles, etc.)
 *
 *  task_func
 *
 *      Task function. It is important to consider the type of
 *      scheduler that is configured in deciding the structure
 *      of this function, i.e., should it return or not?
 *
 *  id
 *
 *      Task ID. Set by scheduler. 
 *
 *      IMPORTANT:
 *      Early versions of this scheduler will step the task id
 *      at an incremement of 1 starting from 0. It is NOT safe
 *      to assume that this will always be the case. The user
 *      should assume that the task id value itself has no use
 *      other than to interface with the simple scheduler.
 *
 */

typedef struct
    {
    uint32_t period_ms;
    void (*task_func)(void);
    sched_task_id_t id;
    } sched_usr_tsk_t;

typedef uint8_t sched_err_t;
enum
{
    SCHED_ERR_NO_ERR,           /* No error */
    SCHED_ERR_FAILED_UPDATE,    /* Failed to update task */
    SCHED_ERR_FAILED_REG,       /* Failed to register task */
    SCHED_ERR_PARAM,            /* Invalid parameters */
    SCHED_ERR_INVLD_STATE,      /* Scheduler is in an invalid state */
};

/*
    Contracted scheduler functions. These must be implemented
    by scheduler implementation.
 */

/**********************************************************
 *
 *  sched_init()
 *
 *  DESCRIPTION:
 *     Initialization function. Called during kernel init.
 *
 */

sched_err_t sched_init(sched_usr_tsk_t *tasks, uint32_t num_tasks);

/**********************************************************
 *
 *  sched_register_task()
 *
 *  DESCRIPTION:
 *     TODO
 *
 *     Called in user and kernel space to register a task.
 *     eventually, there will be a distinction between these
 *
 */

sched_err_t sched_register_task(sched_usr_tsk_t * task);

/**********************************************************
 *
 *  sched_kill_task()
 *
 *  DESCRIPTION:
 *      Kill a task.
 *
 */

sched_err_t sched_kill_task(sched_task_id_t task_id);

/**********************************************************
 *
 *  sched_kill_task()
 *
 *  DESCRIPTION:
 *      alive a task.
 *
 */

sched_err_t sched_alive_task(sched_task_id_t task_id);


/**********************************************************
 *
 *  sched_main()
 *
 *  DESCRIPTION:
 *      Main scheduling function. Control is passed to this
 *      function after kernel is fully booted.
 *
 *  NOTES:
 *      Implementation can do whatever it deems necessary here.
 *
 */

void sched_main();