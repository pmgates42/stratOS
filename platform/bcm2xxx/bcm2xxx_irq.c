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

/* Interrupt source defines*/

#define IRQ_SYS_TIMER_0	(1 << 0)    /* System timer 0 */
#define IRQ_SYS_TIMER_1	(1 << 1)    /* System timer 1 */
#define IRQ_SYS_TIMER_2	(1 << 2)    /* System timer 2 */
#define IRQ_SYS_TIMER_3	(1 << 3)    /* System timer 3 */
#define IRQ_USB_CTRL    (1 << 9)    /* USB contoller */

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

/**********************************************************
 * 
 *  irq_sys_enable
 * 
 * 
 *  DESCRIPTION:
 *      Enable system IRQs. This is called by the kernel
 *      initialization logic after timers and irqs are
 *      initialized.
 * 
 */

void irq_sys_enable(void)
{
    /* Enable peripherals */
    en_periph(BCM2XXX_IRQ_PERIPH_SYS_TMR_M1);

    /* Enable IRQs on the CPU */
    vector_enable_irq();

}

/**********************************************************
 * 
 *  irq_enable_usb
 * 
 * 
 *  DESCRIPTION:
 *      Contracted Enable USB interrupt procedure.
 * 
 */
boolean irq_enable_usb(void)
{
    en_periph(BCM2XXX_IRQ_PERIPH_USB_CTRL);

    return TRUE;
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
    reg32_t irq = REG_IRQ_BASE->irq_pending[0];
    switch (irq)
    {

    /* Handle System timer IRQS */
    case IRQ_SYS_TIMER_0:
    case IRQ_SYS_TIMER_1:
    case IRQ_SYS_TIMER_2:
    case IRQ_SYS_TIMER_3:
        bcm2xxx_timer_irq_hndlr(map_irq_to_timer_channel(irq));
        break;

    /* Handle USB Controller IRQs */
    case IRQ_USB_CTRL:
        printf("USB Controller interrupt");
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
    case IRQ_SYS_TIMER_0:
        return BM2XXX_TIMER_CHNL_0;

    case IRQ_SYS_TIMER_1:
        return BM2XXX_TIMER_CHNL_1;

    case IRQ_SYS_TIMER_2:
        return BM2XXX_TIMER_CHNL_2;

    case IRQ_SYS_TIMER_3:
        return BM2XXX_TIMER_CHNL_3;

    default:
        // Handle unknown IRQ value, or return a default value.
        return BCMXXX_TIMER_CHNL_COUNT;
    }
}
