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

#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)

typedef struct
{
    reg32_t  basic_pending;
    uint64_t irq_pending;
    reg32_t  fiq_ctrl;
    reg32_t  en[2];
    reg32_t  en_basic;
    reg32_t  dis[2];
    reg32_t  dis_basic;

} irq_reg_t;

/* Constants */

#define REG_IRQ_BASE ((volatile irq_reg_t *)(PBASE + 0x0000B200))

static bcm2xxx_timer_t8 map_irq_to_timer_channel(uint8_t irq);

void bcm2xxx_irq_enable(bcm2xxx_irq_periph_t8 periph)
{
    // /* local variables */
    // uint8_t en_reg_idx = 0;

    // /* prevent invalid memory writes */
    // if(periph >= BCM2XXX_IRQ_PERIPH_COUNT)
    //     return;

    // /* clear the interrupt pending enable bit */

    // /* Enable the interrupt */
    // en_reg_idx = periph / 32;
    // REG_IRQ_BASE->irq_pending[en_reg_idx] |= (1 <<periph % 32);
    // REG_IRQ_BASE->en[en_reg_idx]          |= (1 << periph % 32);

}

/* Contracted HW functions */


/**********************************************************
 * 
 *  irq_init.c
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
    uart_send_string("Handling IRQs");//todo remove after test
    uint64_t irq = REG_IRQ_BASE->irq_pending;
    switch (irq)
    {
    /* Handle System timer IRQS */
    case SYSTEM_TIMER_IRQ_0:
    case SYSTEM_TIMER_IRQ_1:
    case SYSTEM_TIMER_IRQ_2:
    case SYSTEM_TIMER_IRQ_3:
        uart_send_string("Handle SYS timer IRQ\n");//todo remove after test
        uart_send((char)map_irq_to_timer_channel(irq));
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