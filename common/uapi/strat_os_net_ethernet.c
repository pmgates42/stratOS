#include "uapi/strat_os_net_ethernet.h"

error_type STRAT_OS_NET_ETHERNET_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_ethernet_frame_t *frame)
{
	(void)packet;
	(void)frame;

	return ERR_INVALID;
}

error_type STRAT_OS_NET_ETHERNET_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_ethernet_frame_t *frame)
{
	(void)packet;
	(void)frame;

	return ERR_INVALID;
}
