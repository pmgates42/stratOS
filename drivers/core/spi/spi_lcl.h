#pragma once

#include "peripherals/spi.h"

/* iterator values */
#define SPI_MODULE_PIN_ID__FIRST    SPI_MODULE_PIN_ID__CS_0
#define SPI_MODULE_PIN_ID__LAST     SPI_MODULE_PIN_ID__MISO
#define SPI_MODULE_PIN_ID__COUNT    (SPI_MODULE_PIN_ID__LAST + 1)

#define SPI_LCL_MAX_BUFFER_SZ     400 /* bytes */

typedef uint8_t channel_buffer_id_type;

typedef struct
{
/* pins */
    struct
    {
    uint32_t cs_0;
    uint32_t cs_1;
    uint32_t sclk;
    uint32_t miso;
    uint32_t mosi;
    } gpio_pins;

    spi_parameter_config_type
      params;

    boolean config_valid;    

    channel_buffer_id_type tx_id;
    channel_buffer_id_type rx_id;

} hw_intf_data_t;

boolean spi_lcl_get_hw_intf_data(hw_intf_data_t * out_data);
boolean spi_lcl_refresh_state();
void spi_lcl_read_buff(channel_buffer_id_type id, char * out_data, uint8_t size);
void spi_lcl_buffer_empty(channel_buffer_id_type id);
