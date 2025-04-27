
/**********************************************************
 * 
 *  spi_params.c
 * 
 *  DESCRIPTION:
 *     This file manages the SPI driver subsystem. This
 *     module is not hardware specific and just like other
 *     modules in the core/spi code, it operates at the logical
 *     level defering to the platform when necessary.
 * 
 *     Here we are responsible for handling things such as:
 * 
 *     1. Setting SPI parameters such as SCLK, transmit
 *        unit size, etc. These will be sourced from the
 *        implementing platform.
 * 
 *     2. Initializing new slave channels and adjusting
 *        parameters accordingly (again, from the platform)
 *
 *     3. Basicaly anything that a SPI driver can do that is
 *        not platform specific.
 */

#include "generic.h"
#include "peripherals/spi.h"
#include "spi_lcl.h"
#include "config.h"
#include "peripherals/gpio.h"
#include "platform/platform_spi_interface.h"

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
static spi_mode_type device_operating_mode;

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

    device_operating_mode = SPI_OPERATING_MODE_UNSPECIFIED;

    hardware_setup();
    cfg_err = config_setup();

    if(cfg_err != CONFIG_ERR_NONE)
    {
        return SPI_MODULE_ERR__INVALID_CONFIG;
    }

    /* Chip selects are active low */
    gpio_set(intf_data.gpio_pins.cs_0);
    gpio_set(intf_data.gpio_pins.cs_1);

    /*  only support a single channel right now */
    intf_data.tx_id = TX_BUFFER_CHANNEL_0;
    intf_data.rx_id = RX_BUFFER_CHANNEL_0;

    //TODO remove this after testing
    /* copy some test data into the tx buffer */
    tx_buffer_channel_0[0] = 0xF0;
    tx_buffer_channel_0[1] = 0xA5;

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
    config_block_t read_block = { 0 };

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

    cfg_err |= config_read(CONFIG_ID_SPI_PARAMS, &read_block);

    if(cfg_err == CONFIG_ERR_NONE)
    {
        memcpy(&intf_data.params, &read_block.spi_params, sizeof intf_data.params);
    }

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
    uint8_t cs_count;   /* chip select count */

    /* Platform specifc initialization */

    cs_count = PLATFORM_spi_init();
    assert( cs_count == 1 || cs_count == 2, "Platform SPI Chips Select pins registered must be either 1 or 2." );

    if( PLATFORM_spi_get_cs_operating_mode() != CHANNEL_OPERATING_MODE__STANDARD )
        {
        assert( FALSE, "Only standard operating mode is supported currently." );
        }

    device_operating_mode = PLATFORM_spi_get_operating_mode();
    assert( device_operating_mode == SPI_OPERATING_MODE__MASTER
           || device_operating_mode == SPI_OPERATING_MODE__SLAVE,
           "Invalid operating mode!" );
}

/* Ensure pins were correctly registered by the platform */
static boolean pins_are_registered(void)
{
    uint8_t pin;

    for(pin = SPI_MODULE_PIN_ID__FIRST; pin < SPI_MODULE_PIN_ID__COUNT; pin++)
    {
        if( !config_pin_is_registered( CONFIG_MODULE_ID__SPI,  pin) )
        {
            return FALSE;
        }
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
