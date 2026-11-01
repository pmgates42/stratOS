/**********************************************************
 *
 *  spi_xfer.c
 *
 *  DESCRIPTION:
 *     Synchronous, full-duplex SPI transaction API on top
 *     of the bit-banged pins owned by the core SPI module.
 *
 *     A transaction is spi_begin() (CS low), any number of
 *     spi_xfer() calls, then spi_end() (CS high). Bytes are
 *     shifted MSB first in SPI mode 0: SCLK idles low, MOSI
 *     is set up before the rising edge and MISO is sampled
 *     on the rising edge.
 *
 *     Only chip select 0 is supported.
 *
 */

#include "generic.h"
#include "peripherals/gpio.h"
#include "peripherals/spi.h"
#include "spi_lcl.h"
#include "utils.h"
#include "debug.h"

/* Half clock period in microseconds. 0 removes the delay
   entirely; the W5500 tolerates up to 80 MHz so a GPIO
   bit-bang can never clock too fast for it. */
#ifndef SPI_BITBANG_HALF_PERIOD_US
#define SPI_BITBANG_HALF_PERIOD_US 1
#endif

#if SPI_BITBANG_HALF_PERIOD_US > 0
#define half_period_delay() delay_us(SPI_BITBANG_HALF_PERIOD_US)
#else
#define half_period_delay() do { } while(0)
#endif

static boolean load_pins(void);
static uint8_t xfer_byte(uint8_t out);

static hw_intf_data_t s_intf_data;
static boolean        s_pins_loaded;
static boolean        s_in_transaction;

/**********************************************************
 *
 *  spi_begin()
 *
 */

error_type spi_begin(uint8_t cs_id)
{
    if(cs_id != 0)
    {
        return ERR_INVALID;
    }

    if(!load_pins())
    {
        return ERR_INVALID;
    }

    if(s_in_transaction)
    {
        debug_printf("\n[SPI] spi_begin called inside an open transaction");
        return ERR_INVALID;
    }

    gpio_clr(s_intf_data.gpio_pins.sclk);
    gpio_clr(s_intf_data.gpio_pins.cs_0);
    s_in_transaction = TRUE;

    return ERR_NO_ERR;
}

/**********************************************************
 *
 *  spi_xfer()
 *
 */

error_type spi_xfer(const uint8_t *tx, uint8_t *rx, size_t len)
{
    size_t  i;
    uint8_t in;
    uint8_t out;

    if(!s_in_transaction)
    {
        return ERR_INVALID;
    }

    for(i = 0; i < len; i++)
    {
        out = (tx != NULL) ? tx[i] : 0x00U;
        in  = xfer_byte(out);

        if(rx != NULL)
        {
            rx[i] = in;
        }
    }

    return ERR_NO_ERR;
}

/**********************************************************
 *
 *  spi_end()
 *
 */

error_type spi_end(uint8_t cs_id)
{
    if(cs_id != 0)
    {
        return ERR_INVALID;
    }

    if(!s_in_transaction)
    {
        return ERR_INVALID;
    }

    gpio_clr(s_intf_data.gpio_pins.sclk);
    gpio_set(s_intf_data.gpio_pins.cs_0);
    s_in_transaction = FALSE;

    return ERR_NO_ERR;
}

/**********************************************************
 *
 *  load_pins()
 *
 *  DESCRIPTION:
 *     Cache the pin numbers from the SPI manager. Returns
 *     FALSE when the SPI module has not been initialized.
 *
 */

static boolean load_pins(void)
{
    if(s_pins_loaded)
    {
        return TRUE;
    }

    if(!spi_lcl_is_initialized())
    {
        debug_printf("\n[SPI] transfer requested before spi_init()");
        return FALSE;
    }

    clr_mem(&s_intf_data, sizeof s_intf_data);

    if(!spi_lcl_get_hw_intf_data(&s_intf_data))
    {
        return FALSE;
    }

    s_pins_loaded = TRUE;
    return TRUE;
}

/**********************************************************
 *
 *  xfer_byte()
 *
 *  DESCRIPTION:
 *     Shift one byte out on MOSI and one byte in on MISO,
 *     MSB first, SPI mode 0.
 *
 */

static uint8_t xfer_byte(uint8_t out)
{
    uint8_t bit;
    uint8_t in = 0;

    for(bit = 0; bit < 8; bit++)
    {
        if(GET_BIT(out, 7 - bit))
        {
            gpio_set(s_intf_data.gpio_pins.mosi);
        }
        else
        {
            gpio_clr(s_intf_data.gpio_pins.mosi);
        }

        half_period_delay();
        gpio_set(s_intf_data.gpio_pins.sclk);

        in = (uint8_t)(in << 1);
        if(gpio_get(s_intf_data.gpio_pins.miso))
        {
            in |= 1U;
        }

        half_period_delay();
        gpio_clr(s_intf_data.gpio_pins.sclk);
    }

    return in;
}
