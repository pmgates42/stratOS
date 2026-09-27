#pragma once

#include "generic.h"
#include "uapi/strat_os_net_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STRAT_OS_NET_ETHERNET_ADDRESS_SIZE 6U
#define STRAT_OS_NET_ETHERNET_HEADER_SIZE 14U
#define STRAT_OS_NET_ETHERNET_TYPE_IPV4 0x0800U

typedef struct
{
	uint8_t destination[STRAT_OS_NET_ETHERNET_ADDRESS_SIZE];
	uint8_t source[STRAT_OS_NET_ETHERNET_ADDRESS_SIZE];
	uint16_t ether_type;
} STRAT_OS_net_ethernet_frame_t;

error_type STRAT_OS_NET_ETHERNET_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_ethernet_frame_t *frame);

error_type STRAT_OS_NET_ETHERNET_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_ethernet_frame_t *frame);

#ifdef __cplusplus
}
#endif
