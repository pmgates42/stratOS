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

error_type STRAT_OS_NET_UDP_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_udp_datagram_t *datagram);

error_type STRAT_OS_NET_UDP_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_udp_datagram_t *datagram);

#ifdef __cplusplus
}
#endif
