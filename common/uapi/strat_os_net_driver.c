#include "uapi/strat_os_net_driver.h"

error_type STRAT_OS_NET_DRIVER_init(
	STRAT_OS_net_driver_t *driver,
	const STRAT_OS_net_hw_driver_interface_t *hardware,
	STRAT_OS_net_driver_pool_t *pool)
{
	(void)driver;
	(void)hardware;
	(void)pool;

	return ERR_INVALID;
}

error_type STRAT_OS_NET_DRIVER_poll(
	STRAT_OS_net_driver_t *driver)
{
	(void)driver;

	return ERR_INVALID;
}

error_type STRAT_OS_NET_DRIVER_receive(
	STRAT_OS_net_driver_t *driver,
	STRAT_OS_net_driver_rx_item_t **rx_item)
{
	(void)driver;
	(void)rx_item;

	return ERR_INVALID;
}

error_type STRAT_OS_NET_DRIVER_release_rx_item(
	STRAT_OS_net_driver_t *driver,
	STRAT_OS_net_driver_rx_item_t *rx_item)
{
	(void)driver;
	(void)rx_item;

	return ERR_INVALID;
}
