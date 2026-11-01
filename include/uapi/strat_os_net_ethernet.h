/*
 *  strat_os_net_ethernet.h
 *
 *  StratOS Ethernet frame parsing interface
 */

#pragma once

#include "generic.h"
#include "uapi/strat_os_net_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STRAT_OS_NET_ETHERNET_ADDRESS_SIZE 6U
#define STRAT_OS_NET_ETHERNET_HEADER_SIZE 14U
#define STRAT_OS_NET_ETHERNET_DESTINATION_OFFSET 0U
#define STRAT_OS_NET_ETHERNET_SOURCE_OFFSET 6U
#define STRAT_OS_NET_ETHERNET_TYPE_OFFSET 12U
#define STRAT_OS_NET_ETHERNET_TYPE_IPV4 0x0800U

typedef struct
{
	uint8_t destination[STRAT_OS_NET_ETHERNET_ADDRESS_SIZE];
	uint8_t source[STRAT_OS_NET_ETHERNET_ADDRESS_SIZE];
	uint16_t ether_type;
} STRAT_OS_net_ethernet_frame_t;

/*
 * Decode the Ethernet header at the packet's current offset.
 *
 * packet: packet view containing an Ethernet frame
 * frame: output structure receiving destination, source, and EtherType
 *
 * The packet view is not modified. The destination and source addresses are
 * copied from the six-byte Ethernet address fields. EtherType is returned in
 * host byte order and is not checked against supported types; the caller
 * dispatches on it. ERR_INVALID is returned when an argument is null, the
 * active range exceeds capacity, or fewer than 14 active bytes are available.
 * The frame is not modified on error.
 */
error_type STRAT_OS_NET_ETHERNET_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_ethernet_frame_t *frame);

/*
 * Remove the Ethernet header from a packet view.
 *
 * packet: packet view to advance
 * frame: decoded Ethernet frame associated with packet
 *
 * The view is advanced by 14 bytes: offset increases and length decreases.
 * The backing bytes are not copied or cleared. ERR_INVALID is returned, with
 * the view unchanged, when an argument is null, the active range exceeds
 * capacity, or the active packet region does not contain a complete Ethernet
 * header.
 */
error_type STRAT_OS_NET_ETHERNET_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_ethernet_frame_t *frame);

#ifdef __cplusplus
}
#endif
