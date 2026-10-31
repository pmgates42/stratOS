#include "uapi/strat_os_net_ipv4.h"

error_type STRAT_OS_NET_IPV4_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_ipv4_packet_t *ipv4)
{
	(void)packet;
	(void)ipv4;

	return ERR_INVALID;
}

error_type STRAT_OS_NET_IPV4_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_ipv4_packet_t *ipv4)
{
	(void)packet;
	(void)ipv4;

	return ERR_INVALID;
}
