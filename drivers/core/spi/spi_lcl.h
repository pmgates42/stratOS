#pragma once

#include "peripherals/spi.h"

/* iterator values */
#define SPI_MODULE_PIN_ID__FIRST    SPI_MODULE_PIN_ID__CS_0,
#define SPI_MODULE_PIN_ID__LAST     SPI_MODULE_PIN_ID__MISO,
#define SPI_MODULE_PIN_ID__COUNT    (SPI_MODULE_PIN_ID__LAST + 1)

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

} hw_intf_data_t;

boolean spi_lcl_get_hw_intf_data(hw_intf_data_t * out_data);
