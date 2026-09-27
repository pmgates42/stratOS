#include "uapi/strat_os_net_hw.h"

error_type STRAT_OS_NET_HW_packet_init(
	STRAT_OS_net_packet_t *packet,
	uint8_t *buffer,
	size_t capacity)
{
	(void)packet;
	(void)buffer;
	(void)capacity;

	return ERR_INVALID;
}

error_type STRAT_OS_NET_HW_packet_from_rx_descriptor(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_hw_rx_descriptor_t *descriptor)
{
	(void)packet;
	(void)descriptor;

	return ERR_INVALID;
}
