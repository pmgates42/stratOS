
/**********************************************************
 * 
 *  spi_params.c
 * 
 *  DESCRIPTION:
 *     This file manages the SPI driver subsystem. Here
 *     we are responsible for handling things such as:
 * 
 *     1. Setting SPI parameters such as SCLK, transmit
 *        unit size, etc.
 * 
 *     2. Initializing new slave channels and adjusting
 *        parameters accordingly.
 *
 */

#include "generic.h"
#include "peripherals/spi.h"
#include "spi_lcl.h"
#include "config.h"
#include "peripherals/gpio.h"

typedef struct
{
    char *   buff;
    uint32_t index;
} spi_buffer_type;

static boolean pins_are_registered(void);
static config_err_t8 config_setup(void);
static void hardware_setup(void);

static boolean spi_module_reset_state;
static hw_intf_data_t intf_data;
static boolean module_initialized;

enum
{
    TX_BUFFER_CHANNEL_0,
    RX_BUFFER_CHANNEL_0,

    BUFFER_ID_COUNT
};

static char tx_buffer_channel_0[SPI_LCL_MAX_BUFFER_SZ];
static char rx_buffer_channel_0[SPI_LCL_MAX_BUFFER_SZ];

static char * buff_table[] = {
    tx_buffer_channel_0,    /* TX_BUFFER_CHANNEL_0 */
    rx_buffer_channel_0     /* RX_BUFFER_CHANNEL_0 */
};

/**********************************************************
 * 
 *  spi_init()
 * 
 *  DESCRIPTION:
 *     Initialize the SPI module.
 *
 *     For the SPI module to successfully initialize, the
 *     following must be true:
 *  
 *      1. A pin configuration for every pin we need to
 *         for transmissions exists
 * 
 *      2. A SPI parameter configuration exists
 * 
 */

spi_module_error_type spi_init()
{
    config_err_t8 cfg_err;

    spi_module_reset_state = TRUE;

    cfg_err = config_setup();

    if(cfg_err != CONFIG_ERR_NONE)
    {
        return SPI_MODULE_ERR__INVALID_CONFIG;
    }

    hardware_setup();

    /*  only support a single channel right now */
    intf_data.tx_id = TX_BUFFER_CHANNEL_0;
    intf_data.rx_id = RX_BUFFER_CHANNEL_0;

    //TODO remove this after testing
    /* copy some test data into the tx buffer */
    tx_buffer_channel_0[0] = 0xA5;
    tx_buffer_channel_0[1] = 0xF0;

    module_initialized = TRUE;
    return SPI_MODULE_ERR__NONE;
}

/**********************************************************
 * 
 *  config_setup()
 * 
 *  DESCRIPTION:
 *     Read in the initial SPI config. These are subject
 *     to change if, for example, a slave device requires it
 */

static config_err_t8 config_setup(void)
{
    config_err_t8 cfg_err = CONFIG_ERR_NONE;

    /* ensure the necessary pins and parameters have been registered
      with the config module */

    if(!pins_are_registered())
    {
        return SPI_MODULE_ERR__PINS_NOT_CONFIGURED;
    }

    if(!config_get_registration_status(CONFIG_ID_SPI_PARAMS))
    {
        return SPI_MODULE_ERR__PARAMS_NOT_CONFIGURED;
    }
 

    /* Read the config data */

    clr_mem(&intf_data, sizeof intf_data);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__CS_0, &intf_data.gpio_pins.cs_0);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__CS_1, &intf_data.gpio_pins.cs_1);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__MISO, &intf_data.gpio_pins.miso);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__MOSI, &intf_data.gpio_pins.mosi);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__SCLK, &intf_data.gpio_pins.sclk);

    cfg_err |= config_read(CONFIG_ID_SPI_PARAMS, &intf_data.params);

    intf_data.config_valid = (cfg_err == CONFIG_ERR_NONE);

    return cfg_err;
}

/**********************************************************
 * 
 *  hardware_setup()
 * 
 *  DESCRIPTION:
 *     Do any necessary hardware initialization
 */

static void hardware_setup(void)
{
    /* Chip selects are active low */
    gpio_set(intf_data.gpio_pins.cs_0);
    gpio_set(intf_data.gpio_pins.cs_1);
}

static boolean pins_are_registered(void)
{
/* Ensure pins were correctly registered
   
   NOTE: this is done to ensure GPIO pins for the SPI module
         have been set up correctly. This is necessary because
         OS builds/configs may or may not expose the pin config
         for this module to the end consumer project.

         In otherwords, we may be down the callstack from a
         non-prvillaged user task and we want to make sure they
         didn't fudge something up.
*/
    uint8_t pin;

    for(pin = SPI_MODULE_PIN_ID__FIRST; pin < SPI_MODULE_PIN_ID__COUNT; pin++)
    {
        if( !config_pin_is_registered( CONFIG_MODULE_ID__SPI,  pin) )
        {
            return FALSE;
        }
        pin++;
    }

    return TRUE;
}


boolean spi_lcl_get_hw_intf_data(hw_intf_data_t * out_data)
{
    assert(module_initialized, "Failed to initialize SPI module");

    if(!intf_data.config_valid)
    {
        return FALSE;
    }

    memcpy(out_data, &intf_data, sizeof(hw_intf_data_t));
    return TRUE;
}


boolean spi_lcl_refresh_state(void)
{
    return spi_module_reset_state;
}

/**********************************************************
 * 
 *  spi_lcl_read_buff()
 * 
 *  DESCRIPTION:
 *      Reads from the buffer with the given id
 * 
 */

void spi_lcl_read_buff(channel_buffer_id_type id, char * out_data, uint8_t size)
{
    assert(out_data && id < BUFFER_ID_COUNT, "Tried to read from an invalid SPI buffer");

    memcpy(out_data, buff_table[id], size);    /* TODO only read the first size bytes for now */
}
