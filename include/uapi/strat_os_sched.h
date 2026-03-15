/*
 *  strat_os_sched.h
 *
 *  StratOS consumer scheduler interface
 */

#pragma once

#include "generic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint32_t period_ms;
    void (*task_func)(void);
} STRAT_OS_task_type;

typedef uint8_t STRAT_OS_SCHED_err_type;
enum
{
    STRAT_OS_SCHED_ERR__NO_ERROR = 0,
    STRAT_OS_SCHED_ERR__PARAM,
    STRAT_OS_SCHED_ERR__FAILED_REG,
    STRAT_OS_SCHED_ERR__INVALID_STATE,
};

/*
 * Register application tasks and initialize the internal scheduler.
 *
 * tasks: application-owned task list
 * num_tasks: number of entries in tasks
 */
STRAT_OS_SCHED_err_type STRAT_OS_SCHED_init(const STRAT_OS_task_type * tasks, uint32_t num_tasks);

#ifdef __cplusplus
}
#endif
