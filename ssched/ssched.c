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

#ifndef SCHED_TSK_MAX
#define SCHED_TSK_MAX  20
#warning Configuration SCHED_TSK_MAX not set, using default value of 20
#endif

/* Types */

typedef struct
    {
    boolean           cnfgd;    /* Task is configured in the system */
    sched_usr_tsk_t * usr_tsk;  /* User task ptr */

    } task_cb_t;

/* Static variables */
static task_cb_t s_task_list[ SCHED_TSK_MAX ];
static boolean s_sched_running;

static void kick_off_sched(void);
static void cfg_hw_timer(void);

/**********************************************************
 * 
 *  sched_init.h
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

    s_sched_running = FALSE;

    /* make sure uart is initialized so we can debug print */
    if(FALSE == uart_is_init())
    {
        uart_init();
    }

    /* input validation */
    if( NULL == tasks )
        return;

    for(i = 0; i < num_tasks; i++ )
    {
        s_task_list[i].cnfgd = TRUE;
        s_task_list[i].usr_tsk = &tasks[i];
    }

    // cfg_hw_timer();

    kick_off_sched();

    if(FALSE == s_sched_running)
    {
        uart_send_string("Failed to initialize the scheduler");
        //todo send err code uart_send()
    }
}


/**********************************************************
 * 
 *  sched_init.h
 * 
 * 
 *  DESCRIPTION:
 *      Initialize the scheduler
 *
 */

void kick_off_sched(void)
{

/* update scheduler state */
s_sched_running = TRUE;
}
