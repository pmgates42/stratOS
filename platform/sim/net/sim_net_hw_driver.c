#include "sim_net_hw_driver.h"

#include "uapi/strat_os_net_hw.h"

static error_type SIM_NET_HW_DRIVER_receive(
	void *context,
	uint8_t *buffer,
	size_t capacity,
	STRAT_OS_net_hw_rx_descriptor_t *descriptor)
{
	SIM_net_hw_driver_t *hardware = context;

	if (hardware == NULL || buffer == NULL || descriptor == NULL)
		return ERR_INVALID;

	if (!hardware->frame_available)
		return ERR_NO_DATA;

	if (hardware->pending_length > capacity)
		return ERR_INVALID;

	memcpy(buffer, hardware->pending_frame, hardware->pending_length);
	descriptor->buffer = buffer;
	descriptor->capacity = capacity;
	descriptor->length = hardware->pending_length;
	descriptor->complete = TRUE;
	hardware->frame_available = FALSE;

	return ERR_NO_ERR;
}

static error_type SIM_NET_HW_DRIVER_release(
	void *context,
	const STRAT_OS_net_hw_rx_descriptor_t *descriptor)
{
	SIM_net_hw_driver_t *hardware = context;

	if (hardware == NULL || descriptor == NULL)
		return ERR_INVALID;

	hardware->frame_released = TRUE;

	return ERR_NO_ERR;
}

error_type SIM_NET_HW_DRIVER_init(
	SIM_net_hw_driver_t *hardware,
	const uint8_t *initial_frame,
	size_t initial_length,
	STRAT_OS_net_hw_driver_interface_t *interface)
{
	if (hardware == NULL || initial_frame == NULL || initial_length == 0U ||
		interface == NULL)
		return ERR_INVALID;

	hardware->pending_frame = initial_frame;
	hardware->pending_length = initial_length;
	hardware->frame_available = TRUE;
	hardware->frame_released = FALSE;

	interface->context = hardware;
	interface->receive = SIM_NET_HW_DRIVER_receive;
	interface->release = SIM_NET_HW_DRIVER_release;

	return ERR_NO_ERR;
}
