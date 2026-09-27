#pragma once

#include "generic.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STRAT_OS_NET_HW_ETHERNET_MTU 1500U

typedef struct
{
	uint8_t *data;
	size_t capacity;
	size_t offset;
	size_t length;
} STRAT_OS_net_packet_t;

typedef struct
{
	uint8_t *buffer;
	size_t capacity;
	size_t length;
	boolean complete;
} STRAT_OS_net_hw_rx_descriptor_t;

error_type STRAT_OS_NET_HW_packet_init(
	STRAT_OS_net_packet_t *packet,
	uint8_t *buffer,
	size_t capacity);

error_type STRAT_OS_NET_HW_packet_from_rx_descriptor(
	STRAT_OS_net_packet_t *packet,
	const STRAT_OS_net_hw_rx_descriptor_t *descriptor);

#ifdef __cplusplus
}
#endif
