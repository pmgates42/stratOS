/**********************************************************
 * 
 *  kernel.c
 * 
 * 
 *  DESCRIPTION:
 *      Main kernel driver
 *
 */

#include "generic.h"
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
#include "printf.h"

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
 *     Main kernel function.     
 * 
 *  NOTES:
 *      First kernel function to be called by the boot code
 *
 */

void kernel_main()
{
    init();

    printf("Kernel initialized\n\rExecuting in EL%d\n", get_el());
    printf(STRATOS_VERSION);

    /* Echo Rx'd uart data forever */
    while(1)
    {
        uart_send(uart_recv());
        delay(5000);
    }
}

static void tty_task(void)
{
    /* echo back user input */
    while(1)
    {
        uart_send('G');
    }
}

static void init(void)
{
    /* Initialize hardware modules */
    uart_init();
	init_printf(0, putc);
    debug_init();
    irq_init();
    timer_init();

    /* Initialize modules that rely on timers */
    sched_init( task_list, 1 );

    /* Enable IRQs */
    irq_enable();

    /* Set up all of the hw drivers */
    setup_drivers();

    if(SNSR_ERR_NONE != snsr_init())
    {
        printf("\n[error]: Failed to initialize sensor manager\n");
    }

    /* Initialize the network interfaces */
    sock_api_init();
}

/**********************************************************
 * 
 *  setup_drivers()
 * 
 *  DESCRIPTION:
 *     Setup the hardware drivers based on the user defined
 *     configurations.    
 * 
 */

static void setup_drivers(void)
{
    /* Initialize all of the driver managers */
    hc_sr04_intf_init();

    #ifdef HW_DRIVER_HC_SR04
    uart_send_string("HC-SR04 Hardware driver(s) configured....\r\n");
    hc_sr04_intf_reg_intf(hc_sr04_get_reg_intf());
    hc_sr04_init();
    uart_send_string("Successfully registered the HC-SR04 driver....\r\n");
    #endif

}
