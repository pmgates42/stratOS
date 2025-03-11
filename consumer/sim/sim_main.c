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
#include "peripherals/spi.h"

static void init(void);
static void setup_drivers(void);
static boolean register_module_pins(void);

typedef struct
{
    config_module_id_type module_id;            /* software module id number */
    uint16_t              pin;                  /* physical hardware pin     */
    config_pin_id_type    id;                   /* pin lookup id             */
} consumer_module_pin_config_entry_type;
static const consumer_module_pin_config_entry_type consumer_module_pin_config_table[] =
{
    /*           module_id                pin   id                          */
    /* PIN0 */{  CONFIG_MODULE_ID__SPI,   0,    SPI_MODULE_PIN_ID__CS_0    },
    /* PIN1 */{  CONFIG_MODULE_ID__SPI,   1,    SPI_MODULE_PIN_ID__SCLK  },
    /* PIN3 */{  CONFIG_MODULE_ID__SPI,   3,    SPI_MODULE_PIN_ID__MOSI    },
    /* PIN4 */{  CONFIG_MODULE_ID__SPI,   4,    SPI_MODULE_PIN_ID__MISO    },
};

static sched_usr_tsk_t  task_list[] =
    {
    /* period_ms                              task_func      */
    // { 1000,                                  net_proc },
    { SIMULATOR_MAINT_TASK_PERIOD_MS,         gpio_maintenance_task },
    { 500,                                    spi_tx_periodic       }
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
spi_parameter_config_type spi_params;
spi_module_error_type     spi_err;

spi_params.bit_order = CONFIG_BIT_ORDER_MSB;
spi_params.data_size = sizeof(uint8_t);
spi_params.slck_speed_hz = 1;

config_register_spi_parameters(spi_params);

/* register SPI parameters */
spi_err = spi_init();
if( SPI_MODULE_ERR__NONE != spi_err )
{
    printf("There was an error initializing the SPI module. Ensure pins and parameters are set correctly. Err: %d", spi_err);
}
}


static boolean register_module_pins(void)
{
uint8_t       i;
config_err_t8 config_err;

for(i = 0; i < list_cnt( consumer_module_pin_config_table ); i++ )
{
    config_err = config_register_pin_for_module( consumer_module_pin_config_table[i].module_id,
                                                  consumer_module_pin_config_table[i].pin,
                                                  consumer_module_pin_config_table[i].id );

    if( config_err != CONFIG_ERR_NONE )
        {
        return FALSE;
        }
}

return TRUE;
}
