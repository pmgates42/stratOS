/**********************************************************
 *
 *  w5500_fake_bus.c
 *
 *  DESCRIPTION:
 *      See w5500_fake_bus.h.
 *
 */

#include "generic.h"
#include "w5500_fake_bus.h"

static error_type fake_begin(void *context);
static error_type fake_xfer(void *context, const uint8_t *tx, uint8_t *rx, size_t len);
static error_type fake_end(void *context);

static void    log_frame(W5500_fake_bus_t *fake);
static uint8_t read_byte(W5500_fake_bus_t *fake, uint8_t bsb, uint16_t addr);
static void    write_byte(W5500_fake_bus_t *fake, uint8_t bsb, uint16_t addr, uint8_t value);
static uint8_t read_socket_reg(W5500_fake_bus_t *fake, uint8_t sock, uint16_t addr);
static void    write_socket_reg(W5500_fake_bus_t *fake, uint8_t sock, uint16_t addr, uint8_t value);
static void    socket_command(W5500_fake_bus_t *fake, uint8_t sock, uint8_t cmd);
static void    chip_reset(W5500_fake_bus_t *fake);
static uint16_t counter_value(W5500_fake_bus_t *fake, uint16_t reg, uint16_t actual);

/*------------------------------------------------------------
 *  Public helpers
 *------------------------------------------------------------*/

error_type W5500_FAKE_BUS_init(W5500_fake_bus_t *fake, w5500_bus_t *bus)
{
    if(fake == NULL || bus == NULL)
    {
        return ERR_INVALID;
    }

    clr_mem(fake, sizeof *fake);
    fake->version = W5500_VERSIONR_VALUE;
    chip_reset(fake);

    bus->context = fake;
    bus->begin   = fake_begin;
    bus->xfer    = fake_xfer;
    bus->end     = fake_end;

    return ERR_NO_ERR;
}

error_type W5500_FAKE_BUS_inject_frame(W5500_fake_bus_t *fake, const uint8_t *frame, size_t len)
{
    size_t   i;
    uint16_t total;
    uint16_t used;

    if(fake == NULL || frame == NULL || len == 0)
    {
        return ERR_INVALID;
    }

    total = (uint16_t)(len + W5500_RX_LENGTH_PREFIX_SIZE);
    used  = (uint16_t)(fake->rx_wr - fake->rx_rd);

    if((size_t)used + total > W5500_BUFFER_SIZE)
    {
        return ERR_NO_MEM;
    }

    fake->rx_buf[fake->rx_wr & W5500_BUFFER_MASK] = (uint8_t)(total >> 8);
    fake->rx_wr++;
    fake->rx_buf[fake->rx_wr & W5500_BUFFER_MASK] = (uint8_t)(total & 0xFFU);
    fake->rx_wr++;

    for(i = 0; i < len; i++)
    {
        fake->rx_buf[fake->rx_wr & W5500_BUFFER_MASK] = frame[i];
        fake->rx_wr++;
    }

    fake->sock[0][W5500_SN_IR] |= W5500_SN_IR_RECV;

    return ERR_NO_ERR;
}

error_type W5500_FAKE_BUS_read_tx(const W5500_fake_bus_t *fake, uint16_t offset, uint8_t *out, size_t len)
{
    size_t i;

    if(fake == NULL || (out == NULL && len > 0))
    {
        return ERR_INVALID;
    }

    for(i = 0; i < len; i++)
    {
        out[i] = fake->tx_buf[(uint16_t)(offset + i) & W5500_BUFFER_MASK];
    }

    return ERR_NO_ERR;
}

error_type W5500_FAKE_BUS_clear_log(W5500_fake_bus_t *fake)
{
    if(fake == NULL)
    {
        return ERR_INVALID;
    }

    clr_mem(fake->log, sizeof fake->log);
    fake->log_count     = 0;
    fake->recv_commands = 0;
    fake->send_commands = 0;
    fake->begin_calls   = 0;
    fake->end_calls     = 0;

    return ERR_NO_ERR;
}

/*------------------------------------------------------------
 *  Bus callbacks
 *------------------------------------------------------------*/

static error_type fake_begin(void *context)
{
    W5500_fake_bus_t *fake = context;

    if(fake == NULL || fake->selected)
    {
        return ERR_INVALID;
    }

    fake->selected     = TRUE;
    fake->header_count = 0;
    fake->begin_calls++;

    return ERR_NO_ERR;
}

static error_type fake_xfer(void *context, const uint8_t *tx, uint8_t *rx, size_t len)
{
    W5500_fake_bus_t *fake = context;
    size_t  i;
    uint8_t out;
    uint8_t in;

    if(fake == NULL || !fake->selected)
    {
        return ERR_INVALID;
    }

    for(i = 0; i < len; i++)
    {
        out = (tx != NULL) ? tx[i] : 0x00U;
        in  = 0x00U;

        if(fake->header_count < W5500_FRAME_HEADER_SIZE)
        {
            fake->header[fake->header_count++] = out;

            if(fake->header_count == W5500_FRAME_HEADER_SIZE)
            {
                fake->cur_addr = (uint16_t)(((uint16_t)fake->header[0] << 8) | fake->header[1]);
                fake->cur_bsb  = (uint8_t)(fake->header[2] >> 3);
                fake->cur_rwb  = (uint8_t)(fake->header[2] & W5500_RWB_WRITE);
                log_frame(fake);
            }
        }
        else
        {
            if(fake->cur_rwb == W5500_RWB_WRITE)
            {
                write_byte(fake, fake->cur_bsb, fake->cur_addr, out);
            }
            else
            {
                in = read_byte(fake, fake->cur_bsb, fake->cur_addr);
            }

            fake->cur_addr++;

            if(fake->log_count > 0 && fake->log_count <= W5500_FAKE_LOG_MAX)
            {
                fake->log[fake->log_count - 1].len++;
            }
        }

        if(rx != NULL)
        {
            rx[i] = in;
        }
    }

    return ERR_NO_ERR;
}

static error_type fake_end(void *context)
{
    W5500_fake_bus_t *fake = context;

    if(fake == NULL || !fake->selected)
    {
        return ERR_INVALID;
    }

    fake->selected = FALSE;
    fake->end_calls++;

    return ERR_NO_ERR;
}

/*------------------------------------------------------------
 *  Chip model
 *------------------------------------------------------------*/

static void log_frame(W5500_fake_bus_t *fake)
{
    W5500_fake_frame_t *entry;

    if(fake->log_count >= W5500_FAKE_LOG_MAX)
    {
        /* keep counting so tests can detect overflow, but stop recording */
        fake->log_count++;
        return;
    }

    entry          = &fake->log[fake->log_count++];
    entry->bsb     = fake->cur_bsb;
    entry->addr    = fake->cur_addr;
    entry->control = fake->header[2];
    entry->len     = 0;
}

static void chip_reset(W5500_fake_bus_t *fake)
{
    clr_mem(fake->common, sizeof fake->common);
    clr_mem(fake->sock, sizeof fake->sock);
    fake->rx_rd = 0;
    fake->rx_wr = 0;
    fake->tx_rd = 0;
    fake->tx_wr = 0;
    /* power-on defaults the driver relies on being overwritten */
    fake->sock[0][W5500_SN_RXBUF_SIZE] = 2;
    fake->sock[0][W5500_SN_TXBUF_SIZE] = 2;
}

static uint8_t read_byte(W5500_fake_bus_t *fake, uint8_t bsb, uint16_t addr)
{
    uint8_t sock;

    if(bsb == W5500_BSB_COMMON)
    {
        if(addr == W5500_REG_VERSIONR)
        {
            return fake->version;
        }

        return (addr < W5500_FAKE_COMMON_REG_SIZE) ? fake->common[addr] : 0xFFU;
    }

    sock = (uint8_t)((bsb - 1U) / 4U);

    if(sock >= W5500_SOCKET_COUNT)
    {
        return 0xFFU;
    }

    switch(bsb & 0x03U)
    {
        case 1: return read_socket_reg(fake, sock, addr);
        case 2: return (sock == 0) ? fake->tx_buf[addr & W5500_BUFFER_MASK] : 0xFFU;
        case 3: return (sock == 0) ? fake->rx_buf[addr & W5500_BUFFER_MASK] : 0xFFU;
        default: return 0xFFU;
    }
}

static void write_byte(W5500_fake_bus_t *fake, uint8_t bsb, uint16_t addr, uint8_t value)
{
    uint8_t sock;

    if(bsb == W5500_BSB_COMMON)
    {
        if(addr == W5500_REG_MR)
        {
            if(value & W5500_MR_RST)
            {
                chip_reset(fake);
                return;
            }
        }

        if(addr == W5500_REG_VERSIONR)
        {
            return;
        }

        if(addr < W5500_FAKE_COMMON_REG_SIZE)
        {
            fake->common[addr] = value;
        }
        return;
    }

    sock = (uint8_t)((bsb - 1U) / 4U);

    if(sock >= W5500_SOCKET_COUNT)
    {
        return;
    }

    switch(bsb & 0x03U)
    {
        case 1:
            write_socket_reg(fake, sock, addr, value);
            break;
        case 2:
            if(sock == 0)
            {
                fake->tx_buf[addr & W5500_BUFFER_MASK] = value;
            }
            break;
        case 3:
            if(sock == 0)
            {
                fake->rx_buf[addr & W5500_BUFFER_MASK] = value;
            }
            break;
        default:
            break;
    }
}

static uint8_t read_socket_reg(W5500_fake_bus_t *fake, uint8_t sock, uint16_t addr)
{
    uint16_t value;

    if(sock == 0)
    {
        switch(addr)
        {
            case W5500_SN_RX_RSR:
                value = counter_value(fake, W5500_SN_RX_RSR, (uint16_t)(fake->rx_wr - fake->rx_rd));
                return (uint8_t)(value >> 8);
            case W5500_SN_RX_RSR + 1:
                return (uint8_t)(fake->counter_snapshot & 0xFFU);

            case W5500_SN_TX_FSR:
                value = counter_value(fake, W5500_SN_TX_FSR,
                                      (uint16_t)(W5500_BUFFER_SIZE - (uint16_t)(fake->tx_wr - fake->tx_rd)));
                return (uint8_t)(value >> 8);
            case W5500_SN_TX_FSR + 1:
                return (uint8_t)(fake->counter_snapshot & 0xFFU);

            case W5500_SN_RX_RD:     return (uint8_t)(fake->rx_rd >> 8);
            case W5500_SN_RX_RD + 1: return (uint8_t)(fake->rx_rd & 0xFFU);
            case W5500_SN_RX_WR:     return (uint8_t)(fake->rx_wr >> 8);
            case W5500_SN_RX_WR + 1: return (uint8_t)(fake->rx_wr & 0xFFU);
            case W5500_SN_TX_RD:     return (uint8_t)(fake->tx_rd >> 8);
            case W5500_SN_TX_RD + 1: return (uint8_t)(fake->tx_rd & 0xFFU);
            case W5500_SN_TX_WR:     return (uint8_t)(fake->tx_wr >> 8);
            case W5500_SN_TX_WR + 1: return (uint8_t)(fake->tx_wr & 0xFFU);
            default:
                break;
        }
    }

    return (addr < W5500_FAKE_SOCKET_REG_SIZE) ? fake->sock[sock][addr] : 0xFFU;
}

static void write_socket_reg(W5500_fake_bus_t *fake, uint8_t sock, uint16_t addr, uint8_t value)
{
    if(addr >= W5500_FAKE_SOCKET_REG_SIZE)
    {
        return;
    }

    switch(addr)
    {
        case W5500_SN_CR:
            socket_command(fake, sock, value);
            return;

        case W5500_SN_IR:
            fake->sock[sock][W5500_SN_IR] &= (uint8_t)~value;
            return;

        default:
            break;
    }

    if(sock == 0)
    {
        switch(addr)
        {
            case W5500_SN_RX_RD:
                fake->rx_rd = (uint16_t)((fake->rx_rd & 0x00FFU) | ((uint16_t)value << 8));
                return;
            case W5500_SN_RX_RD + 1:
                fake->rx_rd = (uint16_t)((fake->rx_rd & 0xFF00U) | value);
                return;
            case W5500_SN_TX_WR:
                fake->tx_wr = (uint16_t)((fake->tx_wr & 0x00FFU) | ((uint16_t)value << 8));
                return;
            case W5500_SN_TX_WR + 1:
                fake->tx_wr = (uint16_t)((fake->tx_wr & 0xFF00U) | value);
                return;
            default:
                break;
        }
    }

    fake->sock[sock][addr] = value;
}

static void socket_command(W5500_fake_bus_t *fake, uint8_t sock, uint8_t cmd)
{
    uint8_t *regs = fake->sock[sock];

    switch(cmd)
    {
        case W5500_SN_CR_OPEN:
            if((regs[W5500_SN_MR] & W5500_SN_MR_PROTOCOL_MASK) == W5500_SN_MR_MACRAW)
            {
                regs[W5500_SN_SR] = W5500_SN_SR_MACRAW;
            }
            else
            {
                regs[W5500_SN_SR] = W5500_SN_SR_CLOSED;
            }
            break;

        case W5500_SN_CR_CLOSE:
            regs[W5500_SN_SR] = W5500_SN_SR_CLOSED;
            break;

        case W5500_SN_CR_RECV:
            fake->recv_commands++;
            if(sock == 0 && fake->rx_rd == fake->rx_wr)
            {
                regs[W5500_SN_IR] &= (uint8_t)~W5500_SN_IR_RECV;
            }
            break;

        case W5500_SN_CR_SEND:
            fake->send_commands++;
            if(sock == 0)
            {
                fake->tx_rd = fake->tx_wr;
            }
            regs[W5500_SN_IR] |= W5500_SN_IR_SEND_OK;
            break;

        default:
            break;
    }

    /* the chip clears Sn_CR once the command is accepted */
    regs[W5500_SN_CR] = 0;
}

/* Snapshot a 16-bit counter on the high-byte read, optionally perturbed */
static uint16_t counter_value(W5500_fake_bus_t *fake, uint16_t reg, uint16_t actual)
{
    (void)reg;

    if(fake->unstable_counter_reads > 0)
    {
        /* each perturbed read differs from the last, like a counter still moving */
        fake->counter_snapshot = (uint16_t)(actual + fake->unstable_counter_reads);
        fake->unstable_counter_reads--;
    }
    else
    {
        fake->counter_snapshot = actual;
    }

    return fake->counter_snapshot;
}
