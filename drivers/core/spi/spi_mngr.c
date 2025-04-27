
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
#include "utils.h"
#include "debug.h"
typedef struct
{
    char *   buff;
    uint32_t index;
} spi_buffer_type;

static boolean pins_are_registered(void);
static boolean config_setup(void);
static void hardware_setup(void);

static boolean spi_module_reset_state;
static hw_intf_data_t intf_data;
static boolean module_initialized;
static spi_op_mode_type device_operating_mode;

enum
{
    TX_BUFFER_CHANNEL_0,
    RX_BUFFER_CHANNEL_0,
    TX_BUFFER_CHANNEL_1,
    RX_BUFFER_CHANNEL_1,
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

error_type spi_init()
{
    boolean config_success;

    spi_module_reset_state = TRUE;

    device_operating_mode = SPI_OPERATING_MODE_UNSPECIFIED;

    hardware_setup();
    config_success = config_setup();

    if(!config_success)
    {
        return ERR_INVALID;
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
    return ERR_NO_ERR;
}

/**********************************************************
 * 
 *  config_setup()
 * 
 *  DESCRIPTION:
 *     Read in the initial SPI config. These are subject
 *     to change if, for example, a slave device requires it
 *
 */

static boolean config_setup(void)
{
    config_err_t8 cfg_err = CONFIG_ERR_NONE;
    config_block_t read_block = { 0 };

    /* ensure the necessary pins and parameters have been registered
      with the config module */

    if(!pins_are_registered())
    {
        assert(FALSE, "SPI pins were not registered.");
        return FALSE;
    }

    if(!config_get_registration_status(CONFIG_ID_SPI_PARAMS))
    {
        return FALSE;
    }

    /* Read the config data */
    if(config_read(CONFIG_ID_SPI_PARAMS, &read_block) != CONFIG_ERR_NONE)
    {
        assert( FALSE, "Failed to read SPI parameters from the config module" );
        return FALSE;
    }

    /* validate the operational mode and chip select count are both set to
       sane values */

    if(read_block.spi_params.cs_mode != CHANNEL_OPERATING_MODE__STANDARD)
    {
        assert( FALSE, "Only standard chip select mode is supported currently." );
        return FALSE;
    }

    if(read_block.spi_params.op_mode != SPI_OPERATING_MODE__MASTER && read_block.spi_params.op_mode != SPI_OPERATING_MODE__SLAVE )
    {
        assert( FALSE, "Platform has configured an invalid SPI operating mode. Valid operating modes are "
                        ENUM_TO_STR(SPI_OPERATING_MODE__SLAVE) " or " ENUM_TO_STR(SPI_OPERATING_MODE__MASTER) ".");
        return FALSE;
    }

    if(read_block.spi_params.cs_count > 2)
    {
        assert( FALSE, "Chip select count should be less than or equal to 2." );
        return FALSE;
    }

    clr_mem(&intf_data, sizeof intf_data);
    
    /* assume we are using cs0 if only one cs pin is configured */
    if(read_block.spi_params.cs_count == 2)
    {
        cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__CS_0, &intf_data.gpio_pins.cs_0);
        cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__CS_1, &intf_data.gpio_pins.cs_1);
    }
    else
    {
        assert(read_block.spi_params.cs_count == 1, "Invalid chip select count value");

        cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__CS_0, &intf_data.gpio_pins.cs_0);
    }
    
    /* these pins should always be configured */
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__MISO, &intf_data.gpio_pins.miso);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__MOSI, &intf_data.gpio_pins.mosi);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__SCLK, &intf_data.gpio_pins.sclk);

    if(cfg_err != CONFIG_ERR_NONE)
    {
        assert( FALSE, "Pin config was determined to be invalid" );
        return FALSE;
    }

    /* config is valid. copy the params and set valid flag */
    memcpy(&intf_data.params, &read_block.spi_params, sizeof intf_data.params);
    intf_data.config_valid = TRUE;

    return TRUE;
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
    boolean init_success;   /* chip select count */

    /* Platform specifc initialization */

    if( FALSE ==  PLATFORM_spi_init() )
    {
        assert( FALSE, "Platform SPI initialization failed." );
        return;
    }
}

/* Ensure pins were correctly registered by the platform */
static boolean pins_are_registered(void)
{
    #define ERR_MSG(pin) "\n" ENUM_TO_STR(SPI_MODULE_PIN_ID__CS_0) " was not configured by the implementing platform."

    static struct
    {
        config_pin_id_type id;
        const char * msg_str;
    } required_pins[] = 
    {
        { SPI_MODULE_PIN_ID__CS_0, ENUM_TO_STR(SPI_MODULE_PIN_ID__CS_0) },
        { SPI_MODULE_PIN_ID__SCLK, ENUM_TO_STR(SPI_MODULE_PIN_ID__SCLK) },
        { SPI_MODULE_PIN_ID__MOSI, ENUM_TO_STR(SPI_MODULE_PIN_ID__MOSI) },
        { SPI_MODULE_PIN_ID__MISO, ENUM_TO_STR(SPI_MODULE_PIN_ID__MISO) },
    };

    uint8_t i;

    for(i = 0; i < list_cnt(required_pins); i++)
    {
        if(FALSE == config_pin_is_registered( CONFIG_MODULE_ID__SPI,  required_pins[i].id) )
        {
            debug_printf(required_pins[i].msg_str);
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
