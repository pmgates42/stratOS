
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

static boolean pins_are_registered(void);

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
    if(!pins_are_registered())
    {
        return SPI_MODULE_ERR__PINS_NOT_CONFIGURED;
    }

    if(!config_get_registration_status(CONFIG_ID_SPI_PARAMS))
    {
        return SPI_MODULE_ERR__PARAMS_NOT_CONFIGURED;
    }

    return SPI_MODULE_ERR__NONE;
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

for(pin = 0; pin < SPI_MODULE_PIN_ID__COUNT; pin++ )
{
    if( !config_pin_is_registered( CONFIG_MODULE_ID__SPI,  pin) )
    {
        return FALSE;
    }
}

}


boolean spi_lcl_get_hw_intf_data(hw_intf_data_t * out_data)
{
    hw_intf_data_t intf_data;
    config_err_t8 cfg_err = CONFIG_ERR_NONE;
 
    clr_mem(&intf_data, sizeof intf_data);

    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__CS_0, &intf_data.gpio_pins.cs_0);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__CS_1, &intf_data.gpio_pins.cs_1);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__MISO, &intf_data.gpio_pins.miso);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__MOSI, &intf_data.gpio_pins.mosi);
    cfg_err |= config_lookup_pin(SPI_MODULE_PIN_ID__SCLK, &intf_data.gpio_pins.sclk);

    if(cfg_err == CONFIG_ERR_NONE)
    {
        *out_data = intf_data;
    }

    return (cfg_err == CONFIG_ERR_NONE);
}
