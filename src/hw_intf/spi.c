
/**********************************************************
 * 
 *  spi_tx_periodic()
 * 
 *  DESCRIPTION:
 *     SPI periodic transmit procedure
 *
 */

#include "generic.h"
#include "config.h"
#include "peripherals/gpio.h"
#include "peripherals/spi.h"

#define DATA_UNIT_TYPE      uint8_t
#define DATA_UNIT_BYTES_SZ  sizeof( DATA_UNIT_TYPE )

#define MAX_DATA_UNITS_PER_BUFF 1

#define TX_BUFF_SZ      (MAX_DATA_UNITS_PER_BUFF * DATA_UNIT_BYTES_SZ)
#define RX_BUFF_SZ      (MAX_DATA_UNITS_PER_BUFF * DATA_UNIT_BYTES_SZ)

/* iterator values */
#define SPI_MODULE_PIN_ID__FIRST    SPI_MODULE_PIN_ID__CS_0,
#define SPI_MODULE_PIN_ID__LAST     SPI_MODULE_PIN_ID__MISO,
#define SPI_MODULE_PIN_ID__COUNT    (SPI_MODULE_PIN_ID__LAST + 1)

static char   tx_buffer[TX_BUFF_SZ] = { 0xA };
static char * tx_buff_ptr;

static boolean buffer_empty(char * buff_ptr);
static boolean pins_are_registered(void);

spi_module_error_type spi_init()
{
    if(!pins_are_registered)
    {
    return SPI_MODULE_ERR__PINS_NOT_CONFIGURED;
    }
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
         didn't fudge something up down here.
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
