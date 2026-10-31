/*
 *  strat_os_net_driver.h
 *
 *  StratOS network-driver receive-path scaffold
 */

#pragma once

#include "generic.h"
#include "uapi/strat_os_core_utils.h"
#include "uapi/strat_os_net_hw.h"
#include "uapi/strat_os_net_hw_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * One item in the driver's completed receive queue.
 *
 * The list node links the item into the queue. The descriptor describes the
 * packet buffer that a real device or simulator has filled.
 */
typedef struct
{
	STRAT_OS_core_linked_list_node_t node;
	STRAT_OS_net_hw_rx_descriptor_t descriptor;
} STRAT_OS_net_driver_rx_item_t;

/*
 * Caller-provided fixed storage owned by the network driver pool.
 *
 * Each item receives one buffer_capacity-sized region from buffer_storage.
 * The storage remains valid for the lifetime of the network driver.
 */
typedef struct
{
	STRAT_OS_net_driver_rx_item_t *items;
	uint8_t *buffer_storage;
	size_t item_count;
	size_t buffer_capacity;
} STRAT_OS_net_driver_pool_t;

/*
 * Driver state shared by the hardware adapter and network receive pipeline.
 *
 * The queue contains completed descriptors waiting to become packet views.
 * The driver owns the queue links; the caller owns each rx_item and its
 * backing buffer for the lifetime of that item.
 */
typedef struct
{
	STRAT_OS_core_linked_list_node_t available_queue;
	STRAT_OS_core_linked_list_node_t rx_queue;
	size_t available_queue_length;
	size_t rx_queue_length;
	STRAT_OS_net_hw_driver_interface_t hardware;
	STRAT_OS_net_driver_pool_t *pool;
} STRAT_OS_net_driver_t;

/*
 * Initialize the driver, its available-item queue, and its completed RX
 * queue. The supplied pool is fixed storage; this API does not allocate from
 * the heap.
 */
error_type STRAT_OS_NET_DRIVER_init(
	STRAT_OS_net_driver_t *driver,
	const STRAT_OS_net_hw_driver_interface_t *hardware,
	STRAT_OS_net_driver_pool_t *pool);

/*
 * Poll the hardware driver and queue one completed receive descriptor.
 *
 * The network driver obtains an item and buffer from its available pool, then
 * asks the hardware implementation to fill that buffer. ERR_NO_DATA means
 * the hardware currently has no frame to queue. ERR_NO_MEM means the pool has
 * no available item.
 */
error_type STRAT_OS_NET_DRIVER_poll(
	STRAT_OS_net_driver_t *driver);

/* Remove the oldest completed descriptor from the RX queue. */
error_type STRAT_OS_NET_DRIVER_receive(
	STRAT_OS_net_driver_t *driver,
	STRAT_OS_net_driver_rx_item_t **rx_item);

/* Return a processed receive item to the hardware driver's buffer pool. */
error_type STRAT_OS_NET_DRIVER_release_rx_item(
	STRAT_OS_net_driver_t *driver,
	STRAT_OS_net_driver_rx_item_t *rx_item);

#ifdef __cplusplus
}
#endif
