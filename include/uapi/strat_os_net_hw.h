/*
 *  strat_os_net_hw.h
 *
 *  StratOS hardware-independent network packet interface
 */

#pragma once

#include "generic.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STRAT_OS_NET_HW_ETHERNET_MTU 1500U

typedef struct
{
	uint8_t *data;
	size_t capacity;
	size_t offset;
	size_t length;
} STRAT_OS_net_packet_t;

/*
 * Hardware receive descriptor.
 *
 * A descriptor is metadata describing a packet buffer. It is not the packet
 * bytes themselves. A hardware driver or simulated driver uses it to report
 * where received bytes were written and whether those bytes are ready for
 * the network stack.
 *
 * buffer: pointer to storage containing the received bytes
 * capacity: total storage available at buffer
 * length: number of received bytes currently valid in buffer
 * complete: TRUE only after the hardware has finished writing the frame
 *
 * For example, if buffer points to eight received bytes, capacity is 8,
 * length is 8, and complete is TRUE:
 *
 *     buffer:
 *     +------+------+------+------+------+------+------+------+
 *     |  10  |  11  |  12  |  13  |  14  |  15  |  08  |  00  |
 *     +------+------+------+------+------+------+------+------+
 *     <---------------- capacity: 8 ------------------------->
 *
 *     length: 8
 *     complete: TRUE
 *
 * This means there are 8 valid bytes in the buffer and the hardware has
 * finished writing them.
 *
 * The driver checks complete and validates length <= capacity before handing
 * the descriptor to the network stack. The packet conversion function then
 * maps buffer, capacity, and length into STRAT_OS_net_packet_t. The complete
 * flag belongs only to the hardware handoff and is not needed by Ethernet,
 * IPv4, or UDP parsing.
 */
typedef struct
{
	uint8_t *buffer;
	size_t capacity;
	size_t length;
	boolean complete;
} STRAT_OS_net_hw_rx_descriptor_t;

/*
 * Initialize an empty packet view over caller-provided storage.
 *
 * packet: packet view to initialize
 * buffer: backing storage owned by the caller
 * capacity: total number of bytes available in buffer
 *
 * The initialized view has offset and length set to zero. Packet users must
 * preserve the invariant offset + length <= capacity.
 */
error_type STRAT_OS_NET_HW_packet_init(
	STRAT_OS_net_packet_t *packet,
	uint8_t *buffer,
	size_t capacity);

/*
 * Convert a completed receive descriptor into a packet view without copying.
 *
 * packet: packet view to populate
 * descriptor: receive buffer metadata supplied by a network driver
 *
 * The descriptor must be complete and its length must not exceed its
 * capacity. The resulting packet points at descriptor->buffer, starts at
 * offset zero, and exposes descriptor->length bytes. The function returns
 * ERR_INVALID when the descriptor is incomplete or out of bounds.
 */
error_type STRAT_OS_NET_HW_packet_from_rx_descriptor(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_hw_rx_descriptor_t *descriptor);

#ifdef __cplusplus
}
#endif
