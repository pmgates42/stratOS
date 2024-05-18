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

#ifndef SSCHED_TSK_MAX
#define SSCHED_TSK_MAX  20
    #warning Configuration SCHED_TSK_MAX not set, using default value of 20
#endif

/* Types */

typedef struct
    {
    boolean           cnfgd;    /* Task is configured in the system */
    sched_usr_tsk_t   usr_tsk;  /* User task ptr */

    } task_cb_t;

/* Variables */
static task_cb_t s_task_list[ SSCHED_TSK_MAX ];
static boolean s_sched_running;
static timer_id_t8 s_sched_tmr_id;
static void sched_task(void);
static void kick_off_sched(void);

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
    {
        return;
    }

    /* set up internal task list */
    for(i = 0; i < num_tasks; i++ )
    {
        s_task_list[i].cnfgd = TRUE;
        s_task_list[i].usr_tsk = tasks[i];
    }

    kick_off_sched();

    if(FALSE == s_sched_running)
    {
        printf("Failed to initialize the scheduler\n");
    }
}


void sched_task(void)
{
    // debug_toggle_led();
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

static void kick_off_sched(void)
{
    /* allocate a system timer */
    if( TIMER_ERR_NONE != timer_alloc(&s_sched_tmr_id, sched_task, 20000))
    {
        s_sched_running = FALSE;
        return;
    }

    s_sched_running = TRUE;
}
