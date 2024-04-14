/**********************************************************
 * 
 *  bcm2xxx_timer_mngr.c
 * 
 * 
 *  DESCRIPTION:
 *      BCM2XXX Timer interface implementation
 *
 *  NOTES:
 * 
 *      System timers:
 *
 *          BCM2XXX has four 32-bit timer channels which all use
 *          a single 64-bit "free" counter. In this context free
 *          means that the counter does not reset and always counts
 *          upwards thus constantly overflowing. The timer channels
 *          will compare agains the lower 32-bits of this 64-bit
 *          counter.
 * 
 */

#include "generic.h"
#include "bcm2xxx_irq.h"
#include "peripherals/base.h"
#include "bcm2xxx_timer.h"
#include "peripherals/timer.h"
#include "debug.h"
#include "uart.h"

#define NUM_COUNT_REGS 2
#define COUNTER_LO     0
#define COUNTER_HI     1

#define TEST_INTERVAL 200000//todo remove after testing

/* Types */

typedef struct
{
    reg32_t control_sts;                           /* Control/Status register */
    reg32_t counter[ NUM_COUNT_REGS ];             /* Counter registers */
    reg32_t compares[ BCMXXX_TIMER_CHNL_COUNT ];   /* Compare registers */

} sys_timer_add_map_t;

#define REG_SYS_ADD_MAP_BASE ((volatile sys_timer_add_map_t *)(PBASE + 0x003000))

/* Variables */
static void_func_t reg_timer_irq_hnldrs[ BCMXXX_TIMER_CHNL_COUNT ];
static uint32_t cur_val = 0;

void timer_init(void)
{
    /* Local variables */
    uint8_t i;

    /* Initialize the reg timer list */
    for(i =0; i < BCMXXX_TIMER_CHNL_COUNT; i++)
    {
        reg_timer_irq_hnldrs[i] = NULL;
    }
}

/**********************************************************
 * 
 *  timer_alloc
 * 
 *  DESCRIPTION:
 *      Enable a timer
 *
 */
// todo make this take in a hi lvl timer cb which specifies ms and not ticks
uint8_t timer_alloc(timer_id_t8 * timer_id, void_func_t irq_cb, uint32_t ticks)
{
    /* local variables */
    bcm2xxx_timer_t8 bcm_tmr = BM2XXX_TIMER_CHNL_1; //todo add timer resrv table

    /* input validaton */
    if(NULL == timer_id)
        {
        return TIMER_ERR_INVALID_PRMTRS;
        }

    /* set the value to match with the low 32-bits
     * of the free counter for the provided timer
     */
    cur_val = REG_SYS_ADD_MAP_BASE->counter[COUNTER_LO];
    ticks = cur_val + TEST_INTERVAL;
    REG_SYS_ADD_MAP_BASE->compares[bcm_tmr] = ticks;

    /* register the irq callback function */
    reg_timer_irq_hnldrs[bcm_tmr] = irq_cb;

    // todo give this a meaningful value
    *timer_id = 12;

    debug_set_led();
    uart_send_string(" ");
    uart_send_uint32(REG_SYS_ADD_MAP_BASE->compares[bcm_tmr]);
    uart_send_string(" ");

    return TIMER_ERR_NONE;
}


/**********************************************************
 * 
 *  bcm2xxx_timer_irq_hndlr
 * 
 *  DESCRIPTION:
 *      Handle incoming timer IRQs
 *
 */

void bcm2xxx_timer_irq_hndlr(bcm2xxx_timer_t8 timer)
{
    /* input validation */
    if(timer >= BCMXXX_TIMER_CHNL_COUNT)
    {
        return;
    }

    /* prepare the next interval */
    cur_val += TEST_INTERVAL;
    REG_SYS_ADD_MAP_BASE->compares[timer] = cur_val;
    REG_SYS_ADD_MAP_BASE->control_sts |= (1 << timer);

    /* otherwise call the registered irq handler
     * if one exists
     */
    if(NULL != reg_timer_irq_hnldrs[timer])
    {
        reg_timer_irq_hnldrs[timer]();
    }

}
