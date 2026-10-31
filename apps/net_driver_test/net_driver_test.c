#include "generic.h"
#include "application/app_interface.h"

#include "app_test.h"
#include "sim_net_hw_driver.h"
#include "uapi/strat_os_net_driver.h"

void APP_test_setup(void)
{
}

void APP_test_teardown(void)
{
}

static boolean APP_net_driver_test_configure(void)
{
    return TRUE;
}

static void test_driver_initializes_fixed_rx_pool(void)
{
    uint8_t initial_frame[] = { 0x00U };
    STRAT_OS_net_driver_rx_item_t rx_items[2];
    uint8_t buffer_storage[2];
    STRAT_OS_net_driver_pool_t pool = {
        rx_items,
        buffer_storage,
        2U,
        1U
    };
    SIM_net_hw_driver_t simulated_hardware;
    STRAT_OS_net_hw_driver_interface_t hardware;
    STRAT_OS_net_driver_t driver = { 0 };

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        SIM_NET_HW_DRIVER_init(
            &simulated_hardware,
            initial_frame,
            sizeof(initial_frame),
            &hardware));
    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_DRIVER_init(&driver, &hardware, &pool));
    TEST_ASSERT_EQUAL(0U, driver.rx_queue_length);
    TEST_ASSERT_EQUAL(2U, driver.available_queue_length);
}

static void test_hardware_descriptor_enters_network_rx_queue(void)
{
    uint8_t frame_data[] = {
        /* The simulated hardware starts with these bytes already received. */
        0x10U, 0x11U, 0x12U, 0x13U
    };
    STRAT_OS_net_driver_rx_item_t rx_items[2];
    uint8_t buffer_storage[2U * sizeof(frame_data)];
    STRAT_OS_net_driver_pool_t pool = {
        rx_items,
        buffer_storage,
        2U,
        sizeof(frame_data)
    };
    SIM_net_hw_driver_t simulated_hardware;
    STRAT_OS_net_hw_driver_interface_t hardware;
    STRAT_OS_net_driver_t driver = { 0 };
    STRAT_OS_net_driver_rx_item_t *received_item = NULL;

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        SIM_NET_HW_DRIVER_init(
            &simulated_hardware,
            frame_data,
            sizeof(frame_data),
            &hardware));
    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_DRIVER_init(&driver, &hardware, &pool));
    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_DRIVER_poll(&driver));
    TEST_ASSERT_EQUAL(1U, driver.rx_queue_length);
    TEST_ASSERT_EQUAL(1U, driver.available_queue_length);

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_DRIVER_receive(&driver, &received_item));
    TEST_ASSERT_EQUAL_PTR(&rx_items[0], received_item);
    TEST_ASSERT_EQUAL_PTR(buffer_storage, received_item->descriptor.buffer);
    TEST_ASSERT_EQUAL(sizeof(frame_data), received_item->descriptor.length);
    TEST_ASSERT_TRUE(received_item->descriptor.complete);

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_DRIVER_release_rx_item(&driver, received_item));
    TEST_ASSERT_TRUE(simulated_hardware.frame_released);
    TEST_ASSERT_EQUAL(2U, driver.available_queue_length);
}

static void test_invalid_hardware_interface_is_rejected(void)
{
    STRAT_OS_net_driver_t driver = { 0 };
    STRAT_OS_net_hw_driver_interface_t hardware = { 0 };
    STRAT_OS_net_driver_pool_t pool = { 0 };

    TEST_ASSERT_EQUAL(ERR_INVALID,
        STRAT_OS_NET_DRIVER_init(&driver, &hardware, &pool));
}

static boolean APP_net_driver_test_run(void)
{
    int unity_result;

    APP_test_begin();
    APP_test_run(test_driver_initializes_fixed_rx_pool);
    APP_test_run(test_hardware_descriptor_enters_network_rx_queue);
    APP_test_run(test_invalid_hardware_interface_is_rejected);
    unity_result = APP_test_end();
    (void)unity_result;

    return TRUE;
}

boolean APP_sched_configure(void)
{
    return APP_net_driver_test_configure() && APP_net_driver_test_run();
}
