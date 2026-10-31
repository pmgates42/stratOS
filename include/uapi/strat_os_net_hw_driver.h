/*
 *  strat_os_net_hw_driver.h
 *
 *  StratOS network hardware-driver interface
 */

#pragma once

#include "generic.h"
#include "uapi/strat_os_net_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Hardware-specific receive and release operations.
 *
 * An ENC28J60 implementation would use these callbacks to read and recycle
 * hardware receive memory. A simulator can implement the same callbacks by
 * copying a pending device frame into the supplied packet buffer. The
 * network driver does not know which implementation it is calling. The
 * receive callback returns ERR_NO_DATA when no frame is currently available.
 */
typedef struct
{
	void *context;
	error_type (*receive)(
		void *context,
		uint8_t *buffer,
		size_t capacity,
		STRAT_OS_net_hw_rx_descriptor_t *descriptor);
	error_type (*release)(
		void *context,
		const STRAT_OS_net_hw_rx_descriptor_t *descriptor);
} STRAT_OS_net_hw_driver_interface_t;

#ifdef __cplusplus
}
#endif