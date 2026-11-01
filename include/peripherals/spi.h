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


/**********************************************************
 * 
 *  spi_begin()
 * 
 *  DESCRIPTION:
 *     Start a synchronous transaction on chip select cs_id
 *     (CS driven low, SCLK idle low). Only cs_id 0 is
 *     supported. Returns ERR_INVALID for any other chip
 *     select, if spi_init() has not run, or if a transaction
 *     is already open.
 *
 */

error_type spi_begin(uint8_t cs_id);


/**********************************************************
 * 
 *  spi_xfer()
 * 
 *  DESCRIPTION:
 *     Full-duplex transfer of len bytes inside an open
 *     transaction, MSB first, SPI mode 0. tx may be NULL
 *     (0x00 is shifted out); rx may be NULL (input is
 *     discarded). Returns ERR_INVALID when no transaction
 *     is open.
 *
 */

error_type spi_xfer(const uint8_t *tx, uint8_t *rx, size_t len);


/**********************************************************
 * 
 *  spi_end()
 * 
 *  DESCRIPTION:
 *     End the open transaction on cs_id (CS driven high).
 *     Returns ERR_INVALID if cs_id is not 0 or no
 *     transaction is open.
 *
 */

error_type spi_end(uint8_t cs_id);
