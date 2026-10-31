/*
 *  strat_os_net_ipv4.h
 *
 *  StratOS IPv4 packet parsing interface
 */

#pragma once

#include "generic.h"
#include "uapi/strat_os_net_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STRAT_OS_NET_IPV4_MIN_HEADER_SIZE 20U
#define STRAT_OS_NET_IPV4_PROTOCOL_UDP 17U

typedef struct
{
	uint8_t version;
	uint8_t header_length;
	uint8_t protocol;
	uint8_t time_to_live;
	uint16_t total_length;
	uint16_t identification;
	uint16_t flags_and_fragment_offset;
	uint16_t header_checksum;
	uint32_t source_address;
	uint32_t destination_address;
} STRAT_OS_net_ipv4_packet_t;

/*
 * Decode the IPv4 header at the packet's current offset.
 *
 * packet: packet view containing an IPv4 packet
 * ipv4: output structure receiving the decoded header fields
 *
 * The packet view is not modified. Multi-byte fields are returned in host
 * byte order, and header_length is expressed in bytes. The first scaffold
 * supports the non-fragmented IPv4 path. ERR_INVALID is returned for a short,
 * malformed, or fragmented packet.
 */
error_type STRAT_OS_NET_IPV4_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_ipv4_packet_t *ipv4);

/*
 * Remove the IPv4 header from a packet view.
 *
 * packet: packet view to advance
 * ipv4: decoded IPv4 header associated with packet
 *
 * The view is advanced by ipv4->header_length bytes: offset increases and
 * length decreases. The backing bytes are not copied or cleared. ERR_INVALID
 * is returned when the header length is outside the active packet region.
 */
error_type STRAT_OS_NET_IPV4_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_ipv4_packet_t *ipv4);

#ifdef __cplusplus
}
#endif
