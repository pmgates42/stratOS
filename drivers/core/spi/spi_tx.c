
/**********************************************************
 * 
 *  spi_tx.c
 * 
 *  DESCRIPTION:
 *     This file manages SPI transmissions. SPI is a full
 *     duplex protocol so we only need a single procedure
 *     spi_tx_periodic() which must be called frequently
 *     enough to account for all possible incoming data
 *     through every channel configured in the system.
 *
 */

#include "generic.h"
#include "config.h"
#include "peripherals/gpio.h"
#include "peripherals/spi.h"
#include "spi_lcl.h"
#include "utils.h"

#define DATA_UNIT_TYPE      uint8_t
#define DATA_UNIT_BYTES_SZ  sizeof( DATA_UNIT_TYPE )

#define MAX_DATA_UNITS_PER_BUFF 1

#define TX_BUFF_SZ      (MAX_DATA_UNITS_PER_BUFF * DATA_UNIT_BYTES_SZ)
#define RX_BUFF_SZ      (MAX_DATA_UNITS_PER_BUFF * DATA_UNIT_BYTES_SZ)

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
    // TODO PMG put these timing parameters somewhere else
    #define DATA_SETUP_TIME_US  2000000 /* 2 seconds */
    #define SCLK_LOW_TIME_US    1000000 /* 1 second */
    #define SCLK_HI_TIME_US     SCLK_LOW_TIME_US

    #define CS_PIN      intf_data.gpio_pins.cs_0
    #define MOSI_PIN    intf_data.gpio_pins.mosi
    #define MISO_PIN    intf_data.gpio_pins.miso
    #define SCLK_PIN    intf_data.gpio_pins.sclk

    static hw_intf_data_t intf_data;

    uint8_t i, j;
    sint8_t data_idx;
    sint8_t polarity;   /* direction to traverse payload (hinges on current config'd LSB/MSB state) */
    uint8_t byte;       /* byte we are currently working on sending */
    uint8_t tx_buff[SPI_LCL_MAX_BUFFER_SZ];
    sint8_t bto_offst;  /* data transfer bit offset */

    if( spi_lcl_refresh_state() )
    {
        clr_mem(&intf_data, sizeof intf_data); 

        spi_lcl_get_hw_intf_data(&intf_data);

    }

    /* nothing to do if the TX buffer is empty */
    // if( spi_lcl_buffer_empty( intf_data.tx_id ) )
    // {
    //     return;
    // }

    /* set up parameters based on the current device */
    if(intf_data.params.endianness == CONFIG_ENDIAN_BIG)
    {
        data_idx =  (sint8_t)(intf_data.params.data_size - 1);
        polarity =  -1;
    }
    else
    {
        data_idx =  0;
        polarity =  1;
    }

    if(intf_data.params.bit_order == CONFIG_BIT_ORDER_MSB)
    {
        bto_offst =  7;
    }
    else
    {
        bto_offst =  0;
    }

    spi_lcl_read_buff(intf_data.tx_id, tx_buff, intf_data.params.data_size);

    gpio_clr(CS_PIN);
    for(i = 0; i < intf_data.params.data_size; i++)
    {
        assert(data_idx < intf_data.params.data_size && data_idx >= 0, "SPI driver: memory error!");
        byte = tx_buff[data_idx];

        for(j = 0; j < 8; j++)
        {
            uint8_t bit_pos = (intf_data.params.bit_order == CONFIG_BIT_ORDER_MSB) ? (7 - j) : j;

            delay_us(DATA_SETUP_TIME_US);
            gpio_set(SCLK_PIN);

            if(GET_BIT(byte, bit_pos))
            {
                gpio_set(MOSI_PIN);
                gpio_set(DEBUG_PIN);
            }
            else
            {
                gpio_clr(MOSI_PIN);
                gpio_clr(DEBUG_PIN);
            }

            delay_us(SCLK_HI_TIME_US);
            gpio_clr(SCLK_PIN);
        }

        data_idx += polarity;
    }
    gpio_set(CS_PIN);

}

