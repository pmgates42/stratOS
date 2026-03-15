#include "sched.h"
#include "uapi/strat_os_sched.h"
#include "uapi/strat_os_kernel_call.h"

#ifndef STRAT_OS_SCHED_MAX_TASKS
    #ifdef SSCHED_TSK_MAX
        #define STRAT_OS_SCHED_MAX_TASKS SSCHED_TSK_MAX
    #else
        #define STRAT_OS_SCHED_MAX_TASKS 20
    #endif
#endif

static sched_usr_tsk_t internal_task_list[STRAT_OS_SCHED_MAX_TASKS];

STRAT_OS_SCHED_err_type STRAT_OS_SCHED_init(const STRAT_OS_task_type * tasks, uint32_t num_tasks)
{
    uint32_t i;
    sched_err_t sched_err;

    if(num_tasks > 0 && tasks == NULL)
    {
        return STRAT_OS_SCHED_ERR__PARAM;
    }

    if(num_tasks > STRAT_OS_SCHED_MAX_TASKS)
    {
        return STRAT_OS_SCHED_ERR__FAILED_REG;
    }

    for(i = 0; i < num_tasks; i++)
    {
        if(tasks[i].task_func == NULL || tasks[i].period_ms == 0)
        {
            return STRAT_OS_SCHED_ERR__PARAM;
        }

        internal_task_list[i].period_ms = tasks[i].period_ms;
        internal_task_list[i].task_func = tasks[i].task_func;
        internal_task_list[i].id = 0;
    }

    sched_err = STRAT_OS_KERNEL_CALL(sched_init, internal_task_list, num_tasks);

    switch(sched_err)
    {
        case SCHED_ERR_NO_ERR:
            return STRAT_OS_SCHED_ERR__NO_ERROR;
        case SCHED_ERR_PARAM:
            return STRAT_OS_SCHED_ERR__PARAM;
        case SCHED_ERR_FAILED_REG:
            return STRAT_OS_SCHED_ERR__FAILED_REG;
        case SCHED_ERR_INVLD_STATE:
            return STRAT_OS_SCHED_ERR__INVALID_STATE;
        default:
            return STRAT_OS_SCHED_ERR__INVALID_STATE;
    }
}
