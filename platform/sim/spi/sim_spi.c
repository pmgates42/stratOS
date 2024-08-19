/**********************************************************
 *
 *  sim_spi.c
 *
 *  DESCRIPTION:
 *      Simulated SPI module.
 *
 */

#include "config.h"
#include "peripherals/gpio.h"
#include "generic.h"
#include "peripherals/spi.h"
#include "platform/platform_spi_interface.h"

typedef struct
{
    uint16_t              phy_pin;    /* physical hardware pin     */
    config_pin_id_type    os_pin_id;  /* pin lookup id             */
} consumer_module_pin_config_entry_type;
static const consumer_module_pin_config_entry_type consumer_module_pin_config_table[] =
{
    { 0,   SPI_MODULE_PIN_ID__CS_0 },
    { 1,   SPI_MODULE_PIN_ID__SCLK },
    { 2,   SPI_MODULE_PIN_ID__MOSI },
    { 3,   SPI_MODULE_PIN_ID__MISO },
};

boolean PLATFORM_spi_init(void)
{
    uint8_t                   i;
    config_err_t8             config_err;
    spi_parameter_config_type spi_params = { 0 };

    for(i = 0; i < list_cnt( consumer_module_pin_config_table ); i++ )
    {
        config_err = config_register_pin_for_module( CONFIG_MODULE_ID__SPI,
                                                    consumer_module_pin_config_table[i].phy_pin,
                                                    consumer_module_pin_config_table[i].os_pin_id );

        if( config_err != CONFIG_ERR_NONE )
            {
            return FALSE;
            }
    }

    /* Enable the pins and set the appropriate function (In/Out) */
    for(i = 0; i < list_cnt(consumer_module_pin_config_table); i++)
    {
        uint32_t pin;
        pin = consumer_module_pin_config_table[i].phy_pin;
    }

    /* Register SPI parameters */
    spi_params.data_size     = sizeof(uint8_t);
    spi_params.slck_speed_hz = 1;
    spi_params.bit_order     = CONFIG_BIT_ORDER_MSB;
    spi_params.endianness    = CONFIG_ENDIAN_BIG;
    spi_params.op_mode       = SPI_OPERATING_MODE__MASTER;
    spi_params.cs_mode       = CHANNEL_OPERATING_MODE__STANDARD;
    spi_params.cs_count      = 1;
    config_register_spi_parameters(spi_params);

    return TRUE;
}
