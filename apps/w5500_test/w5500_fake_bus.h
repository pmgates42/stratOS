/**********************************************************
 *
 *  w5500_fake_bus.h
 *
 *  DESCRIPTION:
 *      Test double for w5500_bus_t. Models enough of a W5500
 *      (common registers, all eight socket register blocks,
 *      a 16 KB RX buffer and a 16 KB TX buffer for socket 0)
 *      to exercise every driver code path on win-sim without
 *      hardware. Every SPI frame header is logged so tests
 *      can assert the exact address and control byte.
 *
 */

#pragma once

#include "generic.h"
#include "w5500.h"

#define W5500_FAKE_COMMON_REG_SIZE  0x40U
#define W5500_FAKE_SOCKET_REG_SIZE  0x30U
#define W5500_FAKE_LOG_MAX          128U

typedef struct
{
    uint8_t  bsb;
    uint16_t addr;
    uint8_t  control;
    size_t   len;           /* data bytes clocked after the header */
} W5500_fake_frame_t;

typedef struct
{
    /* modelled chip state */
    uint8_t  common[W5500_FAKE_COMMON_REG_SIZE];
    uint8_t  sock[W5500_SOCKET_COUNT][W5500_FAKE_SOCKET_REG_SIZE];
    uint8_t  rx_buf[W5500_BUFFER_SIZE];
    uint8_t  tx_buf[W5500_BUFFER_SIZE];
    uint16_t rx_rd;         /* socket 0 pointers, free running */
    uint16_t rx_wr;
    uint16_t tx_rd;
    uint16_t tx_wr;
    uint8_t  version;       /* value returned for VERSIONR   */

    /* fault injection: first N 16-bit counter reads return actual + N, actual + N-1, ... */
    uint32_t unstable_counter_reads;

    /* observation */
    W5500_fake_frame_t log[W5500_FAKE_LOG_MAX];
    size_t   log_count;
    uint32_t recv_commands;
    uint32_t send_commands;
    uint32_t begin_calls;
    uint32_t end_calls;

    /* in-flight frame decode */
    boolean  selected;
    uint8_t  header[W5500_FRAME_HEADER_SIZE];
    size_t   header_count;
    uint8_t  cur_bsb;
    uint8_t  cur_rwb;
    uint16_t cur_addr;
    uint16_t counter_snapshot;
} W5500_fake_bus_t;

/*
 * Reset the fake to power-on state (VERSIONR = 0x04, everything else
 * zero) and fill bus with callbacks pointing at it. Returns ERR_INVALID
 * when either pointer is NULL.
 */
error_type W5500_FAKE_BUS_init(W5500_fake_bus_t *fake, w5500_bus_t *bus);

/*
 * Append one MACRAW frame to the socket-0 RX buffer: 2-byte big-endian
 * length prefix (len + 2) followed by the frame bytes, advancing rx_wr.
 * The bytes are copied. Returns ERR_INVALID for NULL arguments or a
 * zero length, ERR_NO_MEM if the buffer would overflow.
 */
error_type W5500_FAKE_BUS_inject_frame(W5500_fake_bus_t *fake, const uint8_t *frame, size_t len);

/*
 * Copy len bytes out of the socket-0 TX buffer starting at the 16-bit
 * offset (wraps at 16 KB) into out. Returns ERR_INVALID on NULL.
 */
error_type W5500_FAKE_BUS_read_tx(const W5500_fake_bus_t *fake, uint16_t offset, uint8_t *out, size_t len);

/* Clear the frame log and command counters without touching chip state. */
error_type W5500_FAKE_BUS_clear_log(W5500_fake_bus_t *fake);
