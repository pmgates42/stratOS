/**********************************************************
 * 
 *  sim_timer.c
 * 
 *  DESCRIPTION:
 *      Simluated timer module.
 *
 */

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "generic.h"
#include "peripherals/timer.h"

#define TICKS_PER_USEC 1 /* System timer runs at 1Mhz */
#define TICKS_PER_MS ( 1000 * TICKS_PER_USEC )
#define TICKS_PER_SECOND ( 1000 * TICKS_PER_MS ) 

#define MS_TO_USEC (1000)
#define US_TO_MSEC (1/US_TO_MSEC)

static uint32_t scheduler_proc_rate;

void_func_t scheduler_proc;
void* simulate_shed_timer_isr(void* arg);

/**********************************************************
 * 
 *  timer_init()
 * 
 *  DESCRIPTION:
 *      Initialize timer system
 *
 */

void timer_init()
{

}

/**********************************************************
 * 
 *  timer_alloc()
 * 
 *  DESCRIPTION:
 *      Allocate a timer. NOT IMPLEMENTED.
 *
 */

timer_err_t8 timer_alloc(timer_id_t8 * timer_id, void_func_t irq_cb, uint32_t ticks)
{
    pthread_t thread;

    scheduler_proc = irq_cb;
    scheduler_proc_rate = ticks;

    if (pthread_create(&thread, NULL, simulate_shed_timer_isr, NULL) != 0) {
        printf("Failed to create thread");
        return TIMER_ERR_RESOURCE_UNAVAILABLE;
    }

    return TIMER_ERR_NONE;
}

void* simulate_shed_timer_isr(void* arg) {

    while (1) {
        scheduler_proc();

        usleep(TICKS_PER_USEC * scheduler_proc_rate);
    }
    return NULL;
}
