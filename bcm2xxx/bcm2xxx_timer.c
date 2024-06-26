/**********************************************************
 * 
 *  bcm2xxx_timer.c
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

#define TICKS_PER_USEC 1 /* System timer runs at 1Mhz */
#define TICKS_PER_MS ( 1000 * TICKS_PER_USEC )
#define TICKS_PER_SECOND ( 1000 * TICKS_PER_MS ) 

/* Types */

typedef struct
{
    reg32_t control_sts;                           /* Control/Status register */
    reg32_t counter[ NUM_COUNT_REGS ];             /* Counter registers */
    reg32_t compares[ BCMXXX_TIMER_CHNL_COUNT ];   /* Compare registers */

} sys_timer_add_map_t;

#define REG_SYS_ADD_MAP_BASE ((volatile sys_timer_add_map_t *)(PBASE + 0x003000))

typedef struct
{
    void_func_t irq_cb;
    uint32_t tick_interval;
} timer_ctrl_t;

/* Variables */
static timer_ctrl_t timer_ctrl_block[ BCMXXX_TIMER_CHNL_COUNT ];
static uint32_t cur_val = 0;

void timer_init(void)
{
    /* Initialize the reg timer list */
    clr_mem((uint8_t *)timer_ctrl_block, sizeof(timer_ctrl_t) * BCMXXX_TIMER_CHNL_COUNT);
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
    ticks = cur_val + ticks;
    REG_SYS_ADD_MAP_BASE->compares[bcm_tmr] = ticks;

    /* register the irq callback function */
    timer_ctrl_block[bcm_tmr].irq_cb = irq_cb;
    timer_ctrl_block[bcm_tmr].tick_interval = ticks;

    // todo give this a meaningful value
    *timer_id = 12;

    return TIMER_ERR_NONE;
}


/**********************************************************
 * 
 *  delay_us
 * 
 */

void delay_us(uint32_t us)
{
    uint32_t end_val;
    uint32_t current_val;

    current_val = REG_SYS_ADD_MAP_BASE->counter[COUNTER_LO];

    end_val = current_val + us;

    while( REG_SYS_ADD_MAP_BASE->counter[COUNTER_LO] < end_val )
        ;
}


/**********************************************************
 * 
 *  delay_sec
 * 
 */

void delay_sec(uint32_t sec)
{
    delay_us( TICKS_PER_SECOND * sec );
}


/**********************************************************
 * 
 *  delay_ms
 * 
 */

void delay_ms(uint32_t msec)
{
    delay_us( TICKS_PER_MS * msec );
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
    /* local variables */
    uint32_t ticks;

    /* input validation */
    if(timer >= BCMXXX_TIMER_CHNL_COUNT)
    {
        return;
    }

    /* prepare the next interval */
    cur_val = REG_SYS_ADD_MAP_BASE->counter[COUNTER_LO];
    ticks = cur_val + timer_ctrl_block[timer].tick_interval;
    REG_SYS_ADD_MAP_BASE->compares[timer] = ticks;

    REG_SYS_ADD_MAP_BASE->control_sts |= (1 << timer);

    /* otherwise call the registered irq handler
     * if one exists
     */
    if(NULL != timer_ctrl_block[timer].irq_cb)
    {
        timer_ctrl_block[timer].irq_cb();
    }
}
