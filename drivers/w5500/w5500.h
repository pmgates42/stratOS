/**********************************************************
 *
 *  w5500.h
 *
 *  DESCRIPTION:
 *      WIZnet W5500 Ethernet controller driver. Socket 0 is
 *      run in MACRAW mode so the hardware-independent
 *      network stack sees raw Ethernet frames.
 *
 *      The driver reaches the chip only through w5500_bus_t.
 *      On hardware the bus wraps the core SPI transaction API
 *      (w5500_bus_from_spi); tests supply a fake bus.
 *
 *      All functions return error_type, validate pointer
 *      arguments first and return ERR_INVALID on failure.
 *      Multi-byte register values are big-endian on the wire
 *      and host order in memory.
 *
 */

#pragma once

#include "generic.h"
#include "w5500_regs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define W5500_MAC_SIZE          6U

/* Locally administered default MAC ("STRAT"). Apps may pass their own. */
#define W5500_DEFAULT_MAC       { 0x02U, 0x53U, 0x54U, 0x52U, 0x41U, 0x54U }

/* rst_pin value meaning no reset line is wired */
#define W5500_PIN_NOT_WIRED     0xFFFFFFFFU

/* Bounded waits, in microseconds */
#define W5500_SW_RESET_TIMEOUT_US       100000U
#define W5500_COMMAND_TIMEOUT_US        100000U
#define W5500_SOCKET_OPEN_TIMEOUT_US    100000U

/*
 * Transport used by the driver. begin() asserts chip select,
 * xfer() clocks len bytes full duplex (tx or rx may be NULL),
 * end() releases chip select. context is passed back to each
 * callback unchanged.
 */
typedef struct
{
    void *context;
    error_type (*begin)(void *context);
    error_type (*xfer)(void *context, const uint8_t *tx, uint8_t *rx, size_t len);
    error_type (*end)(void *context);
} w5500_bus_t;

typedef struct
{
    w5500_bus_t bus;
    uint32_t    rst_pin;                /* NET_HW_MODULE_PIN_ID__RST, or W5500_PIN_NOT_WIRED */
    uint8_t     mac[W5500_MAC_SIZE];
    boolean     initialized;
} w5500_t;

/*
 * Fill bus with callbacks that wrap spi_begin/spi_xfer/spi_end
 * on chip select 0. spi_init() must have run before the bus is
 * used. Returns ERR_INVALID when bus is NULL.
 */
error_type w5500_bus_from_spi(w5500_bus_t *bus);

/*
 * Raw register / buffer access. bsb selects the block (see
 * W5500_BSB_*), addr is the 16-bit offset inside that block.
 * data is copied in / out of the caller's buffer. len may be 0.
 * Returns ERR_INVALID for NULL dev, NULL data with len > 0, or
 * an unset bus callback; otherwise the bus error.
 */
error_type w5500_read(w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t *data, size_t len);
error_type w5500_write(w5500_t *dev, uint8_t bsb, uint16_t addr, const uint8_t *data, size_t len);

/* 8-bit and big-endian 16-bit helpers over w5500_read/w5500_write. */
error_type w5500_read8(w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t *value);
error_type w5500_write8(w5500_t *dev, uint8_t bsb, uint16_t addr, uint8_t value);
error_type w5500_read16(w5500_t *dev, uint8_t bsb, uint16_t addr, uint16_t *value);
error_type w5500_write16(w5500_t *dev, uint8_t bsb, uint16_t addr, uint16_t value);

/*
 * Read a free-running 16-bit counter (Sn_TX_FSR, Sn_RX_RSR)
 * repeatedly until two consecutive reads agree.
 */
error_type w5500_read16_stable(w5500_t *dev, uint8_t bsb, uint16_t addr, uint16_t *value);

/*
 * Pulse the RST line low (>= 500 us) and wait for the chip to
 * come up (>= 1 ms). No-op returning ERR_NO_ERR when rst_pin is
 * W5500_PIN_NOT_WIRED.
 */
error_type w5500_hw_reset(w5500_t *dev);

/*
 * Software reset through MR.RST; polls until the bit clears.
 * Returns ERR_INVALID on timeout.
 */
error_type w5500_sw_reset(w5500_t *dev);

/* Read VERSIONR into *version (expect W5500_VERSIONR_VALUE). */
error_type w5500_read_version(w5500_t *dev, uint8_t *version);

/*
 * Reset and configure the chip: hardware reset, software reset,
 * version check, SHAR = mac (6 bytes, copied), all 16 KB of RX and
 * TX buffer memory to socket 0, socket 0 opened in MACRAW mode with
 * MAC filtering. bus is copied into dev. rst_pin is looked up from
 * the config module (NET_HW_MODULE_PIN_ID__RST) and left as
 * W5500_PIN_NOT_WIRED if unregistered. Returns ERR_INVALID on a NULL
 * argument, a bad version, or any timeout; dev->initialized is TRUE
 * only on success.
 */
error_type w5500_init(w5500_t *dev, const w5500_bus_t *bus, const uint8_t mac[W5500_MAC_SIZE]);

/* *up = PHYCFGR.LNK */
error_type w5500_link_up(w5500_t *dev, boolean *up);

#ifdef __cplusplus
}
#endif
