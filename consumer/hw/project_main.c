/**********************************************************
 * 
 *  project_main.c
 * 
 * DESCRIPTION:
 *      Project side code
 * 
 *  NOTE:
 *      Kernel boot procedures are done here but will
 *      eventually be moved elsewhere.
 *
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

typedef struct
{
    config_module_id_type module_id;            /* software module id number */
    uint16_t              pin;                  /* physical hardware pin     */
    config_pin_id_type    id;                   /* pin lookup id             */
} consumer_module_pin_config_entry_type;
static const consumer_module_pin_config_entry_type consumer_module_pin_config_table[] =
{
    /**
     * https://pinout.xyz/
     */
    /*           module_id                pin   id                          */
    /* PIN0 */{  CONFIG_MODULE_ID__SPI,   8,    SPI_MODULE_PIN_ID__CS_0    },
    /* PIN0 */{  CONFIG_MODULE_ID__SPI,   7,    SPI_MODULE_PIN_ID__CS_1    },
    /* PIN1 */{  CONFIG_MODULE_ID__SPI,   11,   SPI_MODULE_PIN_ID__SCLK    },
    /* PIN3 */{  CONFIG_MODULE_ID__SPI,   10,   SPI_MODULE_PIN_ID__MOSI    },
    /* PIN4 */{  CONFIG_MODULE_ID__SPI,   9,    SPI_MODULE_PIN_ID__MISO    },
};

static sched_usr_tsk_t  task_list[] =
    {
    /* period_ms                              task_func      */
    { 500,                                    spi_tx_periodic       }
    };

static void init(void);
static void tty_task(void);
static void setup_drivers(void);
static boolean register_module_pins(void);

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

    printf("\nKernel initialized\n\rExecuting in EL%d\n", get_el());
    printf("Version %s", STRATOS_VERSION);

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
	init_printf(0, putc);
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

    // TODO move this to bottom of this function?
    /* Initialize modules that rely on timers */
    sched_init(task_list, list_cnt(task_list));

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
    /* Initialize OS core drivers */
    usb_core_init();

    /* Initialize all of the driver managers */
    hc_sr04_intf_init();

    /* Configured drivers */

    #ifdef HW_DRIVER_HC_SR04
    printf("\nHC-SR04 Hardware driver(s) configured....\n");
    hc_sr04_intf_reg_intf(hc_sr04_get_reg_intf());
    hc_sr04_init();
    printf("Successfully registered the HC-SR04 driver....\n\n");
    #endif
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
