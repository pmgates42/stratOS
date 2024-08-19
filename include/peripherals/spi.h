#pragma once

#include "generic.h"
#include "config.h"

/**********************************************************
 * 
 *  spi_init()
 * 
 *  DESCRIPTION:
 *     Initialize the SPI module
 *
 */

error_type spi_init(void);


/**********************************************************
 * 
 *  spi_tx_periodic()
 * 
 *  DESCRIPTION:
 *     SPI periodic transmit procedure
 *
 */

void spi_tx_periodic(void);
