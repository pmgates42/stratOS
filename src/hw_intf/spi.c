
/**********************************************************
 * 
 *  spi_tx_periodic()
 * 
 *  DESCRIPTION:
 *     SPI periodic transmit procedure
 *
 */

#include "generic.h"
#include "peripherals/gpio.h"

#define DATA_UNIT_TYPE      uint8_t
#define DATA_UNIT_BYTES_SZ  sizeof( DATA_UNIT_TYPE )

#define MAX_DATA_UNITS_PER_BUFF 1

#define TX_BUFF_SZ      (MAX_DATA_UNITS_PER_BUFF * DATA_UNIT_BYTES_SZ)
#define RX_BUFF_SZ      (MAX_DATA_UNITS_PER_BUFF * DATA_UNIT_BYTES_SZ)


static char   tx_buffer[TX_BUFF_SZ] = { 0xA };
static char * tx_buff_ptr;

static boolean buffer_empty(char * buff_ptr);

/**********************************************************
 * 
 *  spi_tx_periodic()
 * 
 *  DESCRIPTION:
 *     SPI periodic transmit procedure. Transmits a single
 *     data buffer unit (sized according to DATA_UNIT_BYTES_SZ)
 *     per call. SPI is a full duplex protocol so we also RX
 *     here
 *
 */

void spi_tx_periodic(void)
{
static boolean init = FALSE;

DATA_UNIT_TYPE tx_unit; /* unit of data to TX   */
DATA_UNIT_TYPE rx_unit; /* unit of data to RX   */

if( !init )
{
    tx_buff_ptr = tx_buffer;
}

if( buffer_empty( tx_buffer ) )
    return;

/* transmit the next data unit in the tx buffer */

tx_unit = *tx_buff_ptr;

tx_buff_ptr++;

}

static boolean buffer_empty(char * buff_ptr)
{
    return FALSE;
}
