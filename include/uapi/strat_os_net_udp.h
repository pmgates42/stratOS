/*
 *  strat_os_net_udp.h
 *
 *  StratOS UDP datagram parsing interface
 */

#pragma once

#include "generic.h"
#include "uapi/strat_os_net_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint16_t source_port;
	uint16_t destination_port;
	uint16_t checksum;
	uint8_t *payload;
	size_t payload_length;
} STRAT_OS_net_udp_datagram_t;

/*
 * Decode the UDP header at the packet's current offset.
 *
 * packet: packet view containing a UDP datagram
 * datagram: output structure receiving header and payload information
 *
 * The packet view is not modified. Ports, length, and checksum are returned
 * in host byte order. payload points into packet->data; it is not copied.
 * ERR_INVALID is returned when the active region is shorter than the UDP
 * header or when the datagram length is outside the active region.
 */
error_type STRAT_OS_NET_UDP_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_udp_datagram_t *datagram);

/*
 * Remove the UDP header from a packet view.
 *
 * packet: packet view to advance
 * datagram: decoded UDP datagram associated with packet
 *
 * The view is advanced by eight bytes: offset increases and length decreases.
 * The backing bytes are not copied or cleared. ERR_INVALID is returned when
 * the active packet region does not contain a complete UDP header.
 */
error_type STRAT_OS_NET_UDP_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_udp_datagram_t *datagram);

#ifdef __cplusplus
}
#endif
