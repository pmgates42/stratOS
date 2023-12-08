/**********************************************************
 * 
 *  sched.h
 * 
 * 
 *  DESCRIPTION:
 *      Task scheduler interface
 *
 */

#include "generic.h"

typedef struct
    {
    uint32_t    period_ms;
    void        (*task)(void);
    } sched_usr_tsk_t;

void sched_init(sched_usr_tsk_t *tasks, uint32_t num_tasks);