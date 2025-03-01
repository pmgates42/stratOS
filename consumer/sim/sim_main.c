/**********************************************************
 * 
 *  sim_kernel.c
 * 
 *  DESCRIPTION:
 *     Boot procedures fo simulated kernel.
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
#include "peripherals/gpio.h"
#include "config.h"

static void init(void);
static void test_gpio_task(void);
static void setup_drivers(void);
static boolean register_module_pins(void);

#define declare_pin_list(pin_list)  \

enum
{
CONFIG_MODULE_ID__SPI = 0
};

typedef struct
{
    config_module_id_type module_id;
    config_pin_type       pin;
} consumer_module_pin_config_entry_type;
static const consumer_module_pin_config_entry_type consumer_module_pin_config_table[] =
{
    /* PIN0 */{  CONFIG_MODULE_ID__SPI,   0 },
    /* PIN1 */{  CONFIG_MODULE_ID__SPI,   1 },
    /* PIN3 */{  CONFIG_MODULE_ID__SPI,   3 },
    /* PIN4 */{  CONFIG_MODULE_ID__SPI,   4 },
};

static sched_usr_tsk_t  task_list[] =
    {
    /* period_ms                             task_func      */
    { SIMULATOR_MAINT_TASK_PERIOD_MS,        test_gpio_task },
    // { 1000,                                  net_proc },
    { SIMULATOR_MAINT_TASK_PERIOD_MS,         gpio_maintenance_task }
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
    printf("Version %s", STRATOS_VERSION);

    /* Call the main scheduler function */
    sched_main();

    /* If for whatever reason the scheduler gives control
     * back to us, just loop forever. */
    while(1)//TODO CPU optimizations, e.g., wait for interrupt?
        ;
}

static void test_gpio_task(void)
{
static boolean do_set = FALSE;
static boolean task_init = FALSE;

if( !task_init )
{
    printf("Init Test GPIO task");

    gpio_pin_enable( 0 );
    gpio_pin_enable( 1 );

    gpio_pin_setas_outp( 0 );
    gpio_pin_setas_outp( 1 );

    task_init = TRUE;
}

if( do_set )
{
    gpio_set( 0 );
    gpio_set( 1 );

    do_set = FALSE;
}
else
{
    gpio_clr( 0 );
    gpio_clr( 1 );

    do_set = TRUE;
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

    /* Initialize the config module (do
       this before drivers are initialized) */
    config_module_init();
   
    if( FALSE == register_module_pins() )
    {
        printf("Consumer setup: Invalid pin config!");
    }

    /* Set up all of the hw drivers */
    setup_drivers();

    // TODO move this into networking module
    set_static_ip();
    net_init();

    // TODO move this to bottom of this function?
    /* Initialize modules that rely on timers */
    sched_init(task_list, list_cnt(task_list));

    /* Enable system IRQs */
    irq_sys_enable();


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


static boolean register_module_pins(void)
{
uint8_t       i;
config_err_t8 config_err;

for(i = 0; i < list_cnt( consumer_module_pin_config_table ); i++ )
{
    config_err = config_register_pins_for_module( consumer_module_pin_config_table[i].module_id,
                                                  consumer_module_pin_config_table[i].pin );

    if( config_err != CONFIG_ERR_NONE )
        {
        return FALSE;
        }
}

return TRUE;
}
