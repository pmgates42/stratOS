#include "uapi/strat_os_net_ethernet.h"

static boolean has_ethernet_header(const STRAT_OS_net_packet_t *packet);

error_type STRAT_OS_NET_ETHERNET_decode(
	const STRAT_OS_net_packet_t *packet,
	STRAT_OS_net_ethernet_frame_t *frame)
{
	const uint8_t *header;
	uint32_t i;

	if ((frame == NULL) || !has_ethernet_header(packet))
	{
		return ERR_INVALID;
	}

	header = packet->data + packet->offset;

	for (i = 0U; i < STRAT_OS_NET_ETHERNET_ADDRESS_SIZE; i++)
	{
		frame->destination[i] =
			header[STRAT_OS_NET_ETHERNET_DESTINATION_OFFSET + i];
		frame->source[i] = header[STRAT_OS_NET_ETHERNET_SOURCE_OFFSET + i];
	}

	frame->ether_type = (uint16_t)(
		((uint16_t)header[STRAT_OS_NET_ETHERNET_TYPE_OFFSET] << 8) |
		header[STRAT_OS_NET_ETHERNET_TYPE_OFFSET + 1U]);

	return ERR_NO_ERR;
}

error_type STRAT_OS_NET_ETHERNET_remove_header(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_ethernet_frame_t *frame)
{
	if ((frame == NULL) || !has_ethernet_header(packet))
	{
		return ERR_INVALID;
	}

	packet->offset += STRAT_OS_NET_ETHERNET_HEADER_SIZE;
	packet->length -= STRAT_OS_NET_ETHERNET_HEADER_SIZE;

	return ERR_NO_ERR;
}

/* Written to avoid offset + length overflow. */
static boolean has_ethernet_header(const STRAT_OS_net_packet_t *packet)
{
	if ((packet == NULL) || (packet->data == NULL) ||
		(packet->offset > packet->capacity) ||
		(packet->length > (packet->capacity - packet->offset)))
	{
		return FALSE;
	}

	return packet->length >= STRAT_OS_NET_ETHERNET_HEADER_SIZE;
}
