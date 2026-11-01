/**********************************************************
 *
 *  w5500.c
 *
 *  DESCRIPTION:
 *      WIZnet W5500 Ethernet controller driver. See w5500.h
 *      for the API contract and w5500_regs.h for the
 *      register map.
 *
 */

#include "generic.h"
#include "config.h"
#include "peripherals/gpio.h"
#include "peripherals/spi.h"
#include "peripherals/timer.h"
#include "utils.h"
#include "debug.h"
#include "w5500.h"

#define W5500_HW_RESET_LOW_US       1000U   /* datasheet: >= 500 us */
#define W5500_HW_RESET_SETTLE_US    2000U   /* datasheet: >= 1 ms   */

static error_type spi_bus_begin(void *context);
static error_type spi_bus_xfer(void *context, const uint8_t *tx, uint8_t *rx, size_t len);
static error_type spi_bus_end(void *context);

static boolean    bus_is_valid(const w5500_bus_t *bus);
static error_type transfer(w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t rwb,
                           const uint8_t *tx, uint8_t *rx, size_t len);
static boolean    timed_out(uint64_t start_us, uint32_t timeout_us);
static error_type socket_command(w5500_t *dev, uint8_t sock, uint8_t cmd);
static uint32_t   lookup_rst_pin(void);

/*------------------------------------------------------------
 *  Bus backed by the core SPI transaction API (CS0)
 *------------------------------------------------------------*/

error_type w5500_bus_from_spi(w5500_bus_t *bus)
{
    if(bus == NULL)
    {
        return ERR_INVALID;
    }

    bus->context = NULL;
    bus->begin   = spi_bus_begin;
    bus->xfer    = spi_bus_xfer;
    bus->end     = spi_bus_end;

    return ERR_NO_ERR;
}

static error_type spi_bus_begin(void *context)
{
    (void)context;
    return spi_begin(0);
}

static error_type spi_bus_xfer(void *context, const uint8_t *tx, uint8_t *rx, size_t len)
{
    (void)context;
    return spi_xfer(tx, rx, len);
}

static error_type spi_bus_end(void *context)
{
    (void)context;
    return spi_end(0);
}

/*------------------------------------------------------------
 *  Register access
 *------------------------------------------------------------*/

error_type w5500_read(w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t *data, size_t len)
{
    if(dev == NULL || (data == NULL && len > 0))
    {
        return ERR_INVALID;
    }

    return transfer(dev, bsb, addr, W5500_RWB_READ, NULL, data, len);
}

error_type w5500_write(w5500_t *dev, uint8_t bsb, uint16_t addr, const uint8_t *data, size_t len)
{
    if(dev == NULL || (data == NULL && len > 0))
    {
        return ERR_INVALID;
    }

    return transfer(dev, bsb, addr, W5500_RWB_WRITE, data, NULL, len);
}

error_type w5500_read8(w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t *value)
{
    if(dev == NULL || value == NULL)
    {
        return ERR_INVALID;
    }

    return w5500_read(dev, bsb, addr, value, 1);
}

error_type w5500_write8(w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t value)
{
    if(dev == NULL)
    {
        return ERR_INVALID;
    }

    return w5500_write(dev, bsb, addr, &value, 1);
}

error_type w5500_read16(w5500_t *dev, uint8_t bsb, uint16_t addr, uint16_t *value)
{
    error_type err;
    uint8_t    raw[2];

    if(dev == NULL || value == NULL)
    {
        return ERR_INVALID;
    }

    err = w5500_read(dev, bsb, addr, raw, sizeof raw);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    *value = (uint16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    return ERR_NO_ERR;
}

error_type w5500_write16(w5500_t *dev, uint8_t bsb, uint16_t addr, uint16_t value)
{
    uint8_t raw[2];

    if(dev == NULL)
    {
        return ERR_INVALID;
    }

    raw[0] = (uint8_t)(value >> 8);
    raw[1] = (uint8_t)(value & 0xFFU);

    return w5500_write(dev, bsb, addr, raw, sizeof raw);
}

error_type w5500_read16_stable(w5500_t *dev, uint8_t bsb, uint16_t addr, uint16_t *value)
{
    error_type err;
    uint16_t   first;
    uint16_t   second;
    uint64_t   start;

    if(dev == NULL || value == NULL)
    {
        return ERR_INVALID;
    }

    start = timer_get_time_us();

    err = w5500_read16(dev, bsb, addr, &first);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    for(;;)
    {
        err = w5500_read16(dev, bsb, addr, &second);
        if(err != ERR_NO_ERR)
        {
            return err;
        }

        if(first == second)
        {
            *value = first;
            return ERR_NO_ERR;
        }

        if(timed_out(start, W5500_COMMAND_TIMEOUT_US))
        {
            debug_printf("\n[W5500] counter 0x%04x never settled", addr);
            return ERR_INVALID;
        }

        first = second;
    }
}

/*------------------------------------------------------------
 *  Reset / identification / init
 *------------------------------------------------------------*/

error_type w5500_hw_reset(w5500_t *dev)
{
    if(dev == NULL)
    {
        return ERR_INVALID;
    }

    if(dev->rst_pin == W5500_PIN_NOT_WIRED)
    {
        return ERR_NO_ERR;
    }

    gpio_clr(dev->rst_pin);
    delay_us(W5500_HW_RESET_LOW_US);
    gpio_set(dev->rst_pin);
    delay_us(W5500_HW_RESET_SETTLE_US);

    return ERR_NO_ERR;
}

error_type w5500_sw_reset(w5500_t *dev)
{
    error_type err;
    uint8_t    mr;
    uint64_t   start;

    if(dev == NULL)
    {
        return ERR_INVALID;
    }

    err = w5500_write8(dev, W5500_BSB_COMMON, W5500_REG_MR, W5500_MR_RST);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    start = timer_get_time_us();

    for(;;)
    {
        err = w5500_read8(dev, W5500_BSB_COMMON, W5500_REG_MR, &mr);
        if(err != ERR_NO_ERR)
        {
            return err;
        }

        if((mr & W5500_MR_RST) == 0)
        {
            return ERR_NO_ERR;
        }

        if(timed_out(start, W5500_SW_RESET_TIMEOUT_US))
        {
            debug_printf("\n[W5500] software reset timed out (MR=0x%02x)", mr);
            return ERR_INVALID;
        }
    }
}

error_type w5500_read_version(w5500_t *dev, uint8_t *version)
{
    if(dev == NULL || version == NULL)
    {
        return ERR_INVALID;
    }

    return w5500_read8(dev, W5500_BSB_COMMON, W5500_REG_VERSIONR, version);
}

error_type w5500_init(w5500_t *dev, const w5500_bus_t *bus, const uint8_t mac[W5500_MAC_SIZE])
{
    error_type err;
    uint8_t    version;
    uint8_t    sock;
    uint8_t    size_kb;
    uint8_t    status;
    uint64_t   start;

    if(dev == NULL || bus == NULL || mac == NULL)
    {
        return ERR_INVALID;
    }

    if(!bus_is_valid(bus))
    {
        return ERR_INVALID;
    }

    clr_mem(dev, sizeof *dev);
    dev->bus     = *bus;
    dev->rst_pin = lookup_rst_pin();
    memcpy(dev->mac, mac, W5500_MAC_SIZE);

    err = w5500_hw_reset(dev);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    err = w5500_sw_reset(dev);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    err = w5500_read_version(dev, &version);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    if(version != W5500_VERSIONR_VALUE)
    {
        debug_printf("\n[W5500] bad version 0x%02x", version);
        return ERR_INVALID;
    }

    err = w5500_write(dev, W5500_BSB_COMMON, W5500_REG_SHAR, dev->mac, W5500_MAC_SIZE);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    /* Give socket 0 all of the RX and TX buffer memory */
    for(sock = 0; sock < W5500_SOCKET_COUNT; sock++)
    {
        size_kb = (sock == W5500_MACRAW_SOCKET) ? (uint8_t)W5500_BUFFER_SIZE_KB : 0U;

        err = w5500_write8(dev, W5500_BSB_SOCKET_REG(sock), W5500_SN_RXBUF_SIZE, size_kb);
        if(err != ERR_NO_ERR)
        {
            return err;
        }

        err = w5500_write8(dev, W5500_BSB_SOCKET_REG(sock), W5500_SN_TXBUF_SIZE, size_kb);
        if(err != ERR_NO_ERR)
        {
            return err;
        }
    }

    err = w5500_write8(dev, W5500_BSB_SOCKET_REG(W5500_MACRAW_SOCKET), W5500_SN_MR,
                       (uint8_t)(W5500_SN_MR_MACRAW | W5500_SN_MR_MFEN));
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    err = socket_command(dev, W5500_MACRAW_SOCKET, W5500_SN_CR_OPEN);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    start = timer_get_time_us();

    for(;;)
    {
        err = w5500_read8(dev, W5500_BSB_SOCKET_REG(W5500_MACRAW_SOCKET), W5500_SN_SR, &status);
        if(err != ERR_NO_ERR)
        {
            return err;
        }

        if(status == W5500_SN_SR_MACRAW)
        {
            break;
        }

        if(timed_out(start, W5500_SOCKET_OPEN_TIMEOUT_US))
        {
            debug_printf("\n[W5500] socket 0 did not enter MACRAW (Sn_SR=0x%02x)", status);
            return ERR_INVALID;
        }
    }

    dev->initialized = TRUE;
    return ERR_NO_ERR;
}

error_type w5500_link_up(w5500_t *dev, boolean *up)
{
    error_type err;
    uint8_t    phycfgr;

    if(dev == NULL || up == NULL)
    {
        return ERR_INVALID;
    }

    err = w5500_read8(dev, W5500_BSB_COMMON, W5500_REG_PHYCFGR, &phycfgr);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    *up = (phycfgr & W5500_PHYCFGR_LNK) ? TRUE : FALSE;
    return ERR_NO_ERR;
}

/*------------------------------------------------------------
 *  Internals
 *------------------------------------------------------------*/

static boolean bus_is_valid(const w5500_bus_t *bus)
{
    return (bus != NULL && bus->begin != NULL && bus->xfer != NULL && bus->end != NULL);
}

/* One complete SPI frame: 3-byte header then len data bytes, CS held low throughout */
static error_type transfer(w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t rwb,
                           const uint8_t *tx, uint8_t *rx, size_t len)
{
    error_type err;
    error_type end_err;
    uint8_t    header[W5500_FRAME_HEADER_SIZE];

    if(!bus_is_valid(&dev->bus))
    {
        return ERR_INVALID;
    }

    header[0] = (uint8_t)(addr >> 8);
    header[1] = (uint8_t)(addr & 0xFFU);
    header[2] = W5500_CONTROL_BYTE(bsb, rwb);

    err = dev->bus.begin(dev->bus.context);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    err = dev->bus.xfer(dev->bus.context, header, NULL, sizeof header);

    if(err == ERR_NO_ERR && len > 0)
    {
        err = dev->bus.xfer(dev->bus.context, tx, rx, len);
    }

    end_err = dev->bus.end(dev->bus.context);

    return (err != ERR_NO_ERR) ? err : end_err;
}

static boolean timed_out(uint64_t start_us, uint32_t timeout_us)
{
    return ((timer_get_time_us() - start_us) >= (uint64_t)timeout_us) ? TRUE : FALSE;
}

/* Write Sn_CR and wait for the chip to accept it (reads back 0) */
static error_type socket_command(w5500_t *dev, uint8_t sock, uint8_t cmd)
{
    error_type err;
    uint8_t    cr;
    uint64_t   start;

    err = w5500_write8(dev, W5500_BSB_SOCKET_REG(sock), W5500_SN_CR, cmd);
    if(err != ERR_NO_ERR)
    {
        return err;
    }

    start = timer_get_time_us();

    for(;;)
    {
        err = w5500_read8(dev, W5500_BSB_SOCKET_REG(sock), W5500_SN_CR, &cr);
        if(err != ERR_NO_ERR)
        {
            return err;
        }

        if(cr == 0)
        {
            return ERR_NO_ERR;
        }

        if(timed_out(start, W5500_COMMAND_TIMEOUT_US))
        {
            debug_printf("\n[W5500] socket %d command 0x%02x not accepted", sock, cmd);
            return ERR_INVALID;
        }
    }
}

static uint32_t lookup_rst_pin(void)
{
    uint32_t pin = W5500_PIN_NOT_WIRED;

    if(config_lookup_pin(NET_HW_MODULE_PIN_ID__RST, &pin) != CONFIG_ERR_NONE)
    {
        return W5500_PIN_NOT_WIRED;
    }

    if(pin >= CFG_INVALID_PIN_NUMBER)
    {
        return W5500_PIN_NOT_WIRED;
    }

    return pin;
}
