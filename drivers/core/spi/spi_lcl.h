#pragma once

#include "peripherals/spi.h"

/* iterator values */
#define SPI_MODULE_PIN_ID__FIRST    SPI_MODULE_PIN_ID__CS_0,
#define SPI_MODULE_PIN_ID__LAST     SPI_MODULE_PIN_ID__MISO,
#define SPI_MODULE_PIN_ID__COUNT    (SPI_MODULE_PIN_ID__LAST + 1)
