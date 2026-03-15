/**********************************************************
 *
 *  boot.c
 *
 *  DESCRIPTION:
 *      Kernel boot procedures
 */

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
#include "printf.h"
#include "usb.h"
#include "peripherals/spi.h"
#include "config.h"
#include "platform/platform_poweron.h"
#include "application/app_interface.h"

/**********************************************************
 *
 * Kernel Tasks
 *
 */

static void init(void);
static void setup_drivers(void);

/**********************************************************
 *
 *  kernel_main()
 *
 *  DESCRIPTION:
 *     Main kernel entry function.
 *
 *  NOTES:
 *      First StratOS function to be called by the boot code
 *
 */

#include "peripherals/gpio.h"

#ifdef EMBEDDED_BUILD
void kernel_main()
#else
int main()
#endif
{
    gpio_pin_enable(ERROR_PIN);
    gpio_pin_set_func(ERROR_PIN, 1);
    gpio_clr(ERROR_PIN);

    init();

    printf("\nKernel initialized\n\rExecuting in EL%d\n", get_el());
    printf("Version %s", STRATOS_VERSION);

    /* Call the main scheduler function */
    sched_main();

    /* If for whatever reason the scheduler gives control
     * back to us, just loop forever. */
    while(1)//TODO CPU optimizations, e.g., wait for interrupt?
        ;

#if !defined(EMBEDDED_BUILD)
    return 0;
#endif
}

static void init(void)
{
    /* Initialize the foundational hardware modules */
    cpu_init();
    uart_init();

    #ifdef EMBEDDED_BUILD
	init_printf(0, putc);
    #endif

    debug_init();
    irq_init();
    timer_init();

    /* Initialize the config module (do
       this before drivers are initialized) */
    config_module_init();

    /* Let the application provide scheduler tasks. */
    if(FALSE == APP_sched_configure())
    {
        printf("\nFailed to configure app scheduler tasks\n");
    }

    /* Enable system IRQs */
    irq_sys_enable();

    /* Set up all of the hw drivers */
    setup_drivers();

    if(SNSR_ERR_NONE != snsr_init())
    {
        printf("\nFailed to initialize sensor manager\n");
    }

    /* Initialize the network interfaces */
    // sock_api_init();
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
    error_type     spi_err;

    /* Configured drivers */

    #ifdef HW_DRIVER_HC_SR04
    // printf("\nHC-SR04 Hardware driver(s) configured....\n");
    // hc_sr04_intf_reg_intf(hc_sr04_get_reg_intf());
    // hc_sr04_init();
    // printf("Successfully registered the HC-SR04 driver....\n\n");
    #endif

    spi_err = spi_init();

    if( ERR_NO_ERR != spi_err )
    {
        printf("There was an error initializing the SPI module. Ensure pins and parameters are set correctly. Err: %d", spi_err);
    }
}

