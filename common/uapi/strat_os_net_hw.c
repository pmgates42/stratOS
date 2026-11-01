#include "uapi/strat_os_net_hw.h"

error_type STRAT_OS_NET_HW_packet_init(
	STRAT_OS_net_packet_t *packet,
	uint8_t *buffer,
	size_t capacity)
{
	if ((packet == NULL) || (buffer == NULL) || (capacity == 0U))
	{
		return ERR_INVALID;
	}

	packet->data = buffer;
	packet->capacity = capacity;
	packet->offset = 0U;
	packet->length = 0U;

	return ERR_NO_ERR;
}

error_type STRAT_OS_NET_HW_packet_from_rx_descriptor(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_hw_rx_descriptor_t *descriptor)
{
	if ((packet == NULL) || (descriptor == NULL))
	{
		return ERR_INVALID;
	}

	if ((descriptor->buffer == NULL) || (descriptor->capacity == 0U) ||
		(descriptor->complete == FALSE) ||
		(descriptor->length > descriptor->capacity))
	{
		return ERR_INVALID;
	}

	packet->data = descriptor->buffer;
	packet->capacity = descriptor->capacity;
	packet->offset = 0U;
	packet->length = descriptor->length;

	return ERR_NO_ERR;
}
