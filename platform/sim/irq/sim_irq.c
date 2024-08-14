/**********************************************************
 * 
 *  sim_irq.c
 * 
 *  DESCRIPTION:
 *      Simulated IRQ module.
 *
 */

#include "generic.h"

boolean irqs_enabled;

void irq_init()
{
    irqs_enabled = FALSE;
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
    irqs_enabled = TRUE;
}