/**********************************************************
 * 
 *  bcm_2xxx_spi.c
 * 
 *  DESCRIPTION:
 *     This file implements the contracted PLATFORM functions
 *     declared in platform_spi_interface.h.
 */

#include "generic.h"
#include "config.h"
#include "platform/platform_spi_interface.h"
#include "peripherals/gpio.h"
#include "include/bcm2xxx_gpio.h"
#include "peripherals/spi.h"

#define CS_COUNT 1  /* only use a single chip select for now */


typedef struct
{
    uint16_t              phy_pin;    /* physical hardware pin     */
    config_pin_id_type    os_pin_id;  /* pin lookup id             */
    bcm2xxx_gpio_fnc_t    function;   /* pin function (input/output)*/
} consumer_module_pin_config_entry_type;
static const consumer_module_pin_config_entry_type consumer_module_pin_config_table[] =
{
    /**
     * The assumption here is that when we are using the BCM CPU, we are using
     * a RPI with the following pinout. What we should actually do is create a
     * new platform (i.e., RPI3/RPI3B+/RPI4 etc.) which encapsulates all boards
     * with this pinout
     * https://pinout.xyz/
     * https://github.com/pmgates42/stratOS/issues/4
     */
    {  8,    SPI_MODULE_PIN_ID__CS_0, BCM2XXX_GPIO_FUNC_OUTPUT },
    // {  7,    SPI_MODULE_PIN_ID__CS_1, BCM2XXX_GPIO_FUNC_OUTPUT }, // TODO Enable CS1
    {  11,   SPI_MODULE_PIN_ID__SCLK, BCM2XXX_GPIO_FUNC_OUTPUT },
    {  10,   SPI_MODULE_PIN_ID__MOSI, BCM2XXX_GPIO_FUNC_OUTPUT },
    {  9,    SPI_MODULE_PIN_ID__MISO, BCM2XXX_GPIO_FUNC_INPUT  },
};

boolean PLATFORM_spi_init(void)
{
    uint8_t                   i;
    config_err_t8             config_err;
    spi_parameter_config_type spi_params = { 0 };

    /* Register SPI module pins with the config module. The OS SPI driver needs us to do this
       since it performs pin operations via the OS ids (as opossed to the physical pin numbers) */
    for(i = 0; i < list_cnt( consumer_module_pin_config_table ); i++ )
    {
        config_err = config_register_pin_for_module( CONFIG_MODULE_ID__SPI,
                                                    consumer_module_pin_config_table[i].phy_pin,
                                                    consumer_module_pin_config_table[i].os_pin_id );

        if( config_err != CONFIG_ERR_NONE )
            {
            return FALSE; /* no pins registered */
            }
    }

    /* Enable the pins and set the appropriate function (In/Out) */
    for(i = 0; i < list_cnt(consumer_module_pin_config_table); i++)
    {
        uint32_t pin;
        pin = consumer_module_pin_config_table[i].phy_pin;

        gpio_pin_enable(pin);
        gpio_pin_set_func(pin, consumer_module_pin_config_table[i].function);
    }

    /* Register SPI parameters */
    spi_params.data_size     = sizeof(uint8_t);
    spi_params.slck_speed_hz = 1;
    spi_params.bit_order     = CONFIG_BIT_ORDER_MSB;
    spi_params.endianness    = CONFIG_ENDIAN_BIG;
    spi_params.op_mode       = SPI_OPERATING_MODE__MASTER;
    spi_params.cs_mode       = CHANNEL_OPERATING_MODE__STANDARD;
    spi_params.cs_count      = 1; // TODO Enable CS1
    config_register_spi_parameters(spi_params); 

    return TRUE;
}
