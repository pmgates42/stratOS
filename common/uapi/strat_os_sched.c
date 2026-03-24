#include "sched.h"
#include "uapi/strat_os_sched.h"
#include "uapi/strat_os_kernel_call.h"
#include "debug.h"

#ifndef STRAT_OS_SCHED_MAX_TASKS
    #ifdef SSCHED_TSK_MAX
        #define STRAT_OS_SCHED_MAX_TASKS SSCHED_TSK_MAX
    #else
        #define STRAT_OS_SCHED_MAX_TASKS 20
    #endif
#endif

static sched_usr_tsk_t internal_task_list[STRAT_OS_SCHED_MAX_TASKS];

static void debug_log_task_func_ptr(const char * label, void (*task_func)(void))
{
    size_t raw_ptr = (size_t)task_func;
    uint64_t raw_ptr64 = (uint64_t)raw_ptr;
    uint32_t hi = 0U;
    uint32_t lo = (uint32_t)(raw_ptr & 0xFFFFFFFFUL);

    if(sizeof(size_t) > sizeof(uint32_t))
    {
        hi = (uint32_t)((raw_ptr64 >> 32U) & 0xFFFFFFFFULL);
    }

    debug_printf("\n[SCHED] %s=0x%08x%08x", label, (unsigned int)hi, (unsigned int)lo);
}

STRAT_OS_SCHED_err_type STRAT_OS_SCHED_init(const STRAT_OS_task_type * tasks, uint32_t num_tasks)
{
    uint32_t i;
    sched_err_t sched_err;

    if(num_tasks > 0 && tasks == NULL)
    {
        debug_printf("\n[SCHED] Invalid parameters passed to STRAT_OS_SCHED_init");
        return STRAT_OS_SCHED_ERR__PARAM;
    }

    if(num_tasks > STRAT_OS_SCHED_MAX_TASKS)
    {
        debug_printf("\n[SCHED] Too many tasks passed to STRAT_OS_SCHED_init. Max supported is %u", STRAT_OS_SCHED_MAX_TASKS);
        return STRAT_OS_SCHED_ERR__FAILED_REG;
    }

    for(i = 0; i < num_tasks; i++)
    {
        if(tasks[i].task_func == NULL || tasks[i].period_ms == 0)
        {
            debug_printf("\n[SCHED] Invalid task parameters for task %u passed to STRAT_OS_SCHED_init", (unsigned int)i);
            return STRAT_OS_SCHED_ERR__PARAM;
        }

        debug_log_task_func_ptr("tasks[i].task_func", tasks[i].task_func);
        internal_task_list[i].period_ms = tasks[i].period_ms;
        internal_task_list[i].task_func = tasks[i].task_func;
        internal_task_list[i].id = 0;
        debug_log_task_func_ptr("internal_task_list[i].task_func", internal_task_list[i].task_func);
    }

    sched_err = STRAT_OS_KERNEL_CALL(sched_init, internal_task_list, num_tasks);

    if( sched_err != SCHED_ERR_NO_ERR)
    {
        debug_printf("\n[SCHED] STRAT_OS_KERNEL_CALL(sched_init) failed with error code %u", (unsigned int)sched_err);
    }

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
