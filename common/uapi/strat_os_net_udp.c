#include "uapi/strat_os_net_udp.h"

error_type STRAT_OS_NET_UDP_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_udp_datagram_t *datagram)
{
	(void)packet;
	(void)datagram;

	return ERR_INVALID;
}

error_type STRAT_OS_NET_UDP_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_udp_datagram_t *datagram)
{
	(void)packet;
	(void)datagram;

	return ERR_INVALID;
}
