/*
 *  sim_net_hw_driver.h
 *
 *  StratOS simulated network hardware driver
 */

#pragma once

#include "generic.h"
#include "uapi/strat_os_net_hw_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	const uint8_t *pending_frame;
	size_t pending_length;
	boolean frame_available;
	boolean frame_released;
} SIM_net_hw_driver_t;

/*
 * Configure simulated hardware storage and expose its driver callbacks.
 *
 * initial_frame and initial_length describe a frame already waiting in the
 * simulated device when it starts. This models hardware that has received a
 * frame before the network driver begins polling.
 */
error_type SIM_NET_HW_DRIVER_init(
	SIM_net_hw_driver_t *hardware,
	const uint8_t *initial_frame,
	size_t initial_length,
	STRAT_OS_net_hw_driver_interface_t *interface);

#ifdef __cplusplus
}
#endif