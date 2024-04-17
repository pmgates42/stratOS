/**********************************************************
 * 
 *  bcm2xxx_irq.c
 * 
 * 
 *  DESCRIPTION:
 *      BCM2XXX IRQ Operations
 *
 */

#include "generic.h"
#include "bcm2xxx_irq.h"
#include "bcm2xxx_timer.h"
#include "vector.h"
#include "peripherals/base.h"
#include "uart.h"//todo remove after testing
#include "debug.h"//todo remove after testing
#include "printf.h"//todo remove after testing

#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)

typedef struct
{
    reg32_t  basic_pending;
    reg32_t  irq_pending[2];
    reg32_t  fiq_ctrl;
    reg32_t  en[2];
    reg32_t  en_basic;
    reg32_t  dis[2];
    reg32_t  dis_basic;

} irq_reg_t;

/* Constants */

#define REG_IRQ_BASE ((volatile irq_reg_t *)(PBASE + 0x0000B200))

static bcm2xxx_timer_t8 map_irq_to_timer_channel(uint8_t irq);

static void en_periph(bcm2xxx_irq_periph_t8 periph)
{
    /* local variables */
    uint32_t en_reg_idx = 0;

    /* prevent invalid memory writes */
    if(periph >= BCM2XXX_IRQ_PERIPH_COUNT)
        return;

    /* Enable the interrupt */
    en_reg_idx = (uint32_t)periph / 32;
    REG_IRQ_BASE->en[en_reg_idx] |= ( 1 << ( periph % 32 ) );

}

/* Contracted HW functions */


/**********************************************************
 * 
 *  irq_init
 * 
 * 
 *  DESCRIPTION:
 *      Initialize IRQs.
 * 
 *  NOTES:
 *      We will also initialize the vector table here using
 *      the common AArch64 vector initialization process.
 *      The IRQ vector entry will eventually throw control
 *      back to us when we need to handle incoming IRQs
 *
 */

void irq_init(void)
{
    /* Initialize the exception vector table */
    vector_init();
}

void irq_enable()
{
    /* Enable peripherals */
    en_periph(BCM2XXX_IRQ_PERIPH_SYS_TMR_M1);

    /* Enable IRQs on the CPU */
    vector_enable_irq();

}

/**********************************************************
 * 
 *  irq_handle_irqs
 * 
 * 
 *  DESCRIPTION:
 *      Handle incoming IRQs.
 * 
 *  NOTES:
 *      Should only be called by AArch64 vector table mapped
 *      IRQ handler.
 *
 */

void irq_handle_irqs(void)
{
    uint64_t irq = REG_IRQ_BASE->irq_pending[0];
    switch (irq)
    {
    /* Handle System timer IRQS */
    case SYSTEM_TIMER_IRQ_0:
    case SYSTEM_TIMER_IRQ_1:
    case SYSTEM_TIMER_IRQ_2:
    case SYSTEM_TIMER_IRQ_3:
        bcm2xxx_timer_irq_hndlr(map_irq_to_timer_channel(irq));
        break;
    default:
        break;
    }
}

/**********************************************************
 * 
 *  map_irq_to_timer_channel
 * 
 * 
 *  DESCRIPTION:
 *      Map the IRQ System timer channel bit to the 
 *      the timer channel index.
 *
 */

static bcm2xxx_timer_t8 map_irq_to_timer_channel(uint8_t irq)
{
    switch (irq)
    {
    case SYSTEM_TIMER_IRQ_0:
        return BM2XXX_TIMER_CHNL_0;

    case SYSTEM_TIMER_IRQ_1:
        return BM2XXX_TIMER_CHNL_1;

    case SYSTEM_TIMER_IRQ_2:
        return BM2XXX_TIMER_CHNL_2;

    case SYSTEM_TIMER_IRQ_3:
        return BM2XXX_TIMER_CHNL_3;

    default:
        // Handle unknown IRQ value, or return a default value.
        return BCMXXX_TIMER_CHNL_COUNT;
    }
}
