#pragma once

#include "generic.h"
#include "config.h"

/**********************************************************
 * 
 *  spi_module_error_type
 * 
 *  DESCRIPTION:
 *     Errors returned by the SPI module
 *
 */

typedef uint8_t spi_module_error_type;
enum
{
SPI_MODULE_ERR__NONE,
SPI_MODULE_ERR__PINS_NOT_CONFIGURED,    /* GPIO Pins for this module were not configured    */
SPI_MODULE_ERR__PARAMS_NOT_CONFIGURED,  /* Parameters, e.g., SLCK rate, data size, etc. are not configured */
SPI_MODULE_ERR__INVALID_CONFIG,         /* SPI configuration is invalid */
};


/**********************************************************
 * 
 *  spi_init()
 * 
 *  DESCRIPTION:
 *     Initialize the SPI module
 *
 */

spi_module_error_type spi_init(void);


/**********************************************************
 * 
 *  spi_tx_periodic()
 * 
 *  DESCRIPTION:
 *     SPI periodic transmit procedure
 *
 */

void spi_tx_periodic(void);
