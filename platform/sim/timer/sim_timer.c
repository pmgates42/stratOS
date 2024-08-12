/**********************************************************
 * 
 *  sim_timer.c
 * 
 *  DESCRIPTION:
 *      Simluated timer module.
 *
 */

#include "generic.h"
#include "peripherals/timer.h"

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
    return TIMER_ERR_NONE;
}
