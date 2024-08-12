/**********************************************************
 * 
 *  sim_kernel.c
 * 
 *  DESCRIPTION:
 *     Boot procedures fo simulated kernel
 *
 */

#include <stdio.h>
#include "generic.h"
#include "cpu.h"
#include "net.h"
#include "uart.h"
#include "sched.h"
#include "irq.h"
#include "kernel.h"
#include "peripherals/timer.h"
#include "peripherals/snsr/snsr.h"
#include "peripherals/drivers/hc_sr04.h"
#include "debug.h"
#include "utils.h"
#include "usb.h"

static void init(void);
static void tty_task(void);
static void setup_drivers(void);

static sched_usr_tsk_t task_list[] =
    {
    { 10 /* ms */, tty_task }
    };

/**********************************************************
 * 
 *  kernel_main()
 * 
 *  DESCRIPTION:
 *     Main (sim) kernel function.
 *
 */

void /*kernel_*/main()
{
    init();

    printf("\nKernel initialized\n\rExecuting in EL%d\n", get_el());
    printf("Version %d", STRATOS_VERSION);

    /* Call the main scheduler function */
    sched_main();

    /* If for whatever reason the scheduler gives control
     * back to us, just loop forever. */
    while(1)//TODO CPU optimizations, e.g., wait for interrupt?
        ;
}

static void tty_task(void)
{
    /* echo back user input */
    while(1)
    {
        debug_toggle_led();
        // uart_send(uart_recv());
    }
}

static void init(void)
{
    /* Initialize hardware modules */
    cpu_init();
    uart_init();
    debug_init();
    irq_init();
    timer_init();

    // TODO move this to bottom of this function?
    /* Initialize modules that rely on timers */
    sched_init(task_list, list_cnt(task_list));

    /* Enable system IRQs */
    irq_sys_enable();

    /* Set up all of the hw drivers */
    setup_drivers();
}

/**********************************************************
 * 
 *  setup_drivers()
 * 
 *  DESCRIPTION:
 *     Setup the hardware drivers
 * 
 */

static void setup_drivers(void)
{

}
