#include "generic.h"
#include "application/app_interface.h"

#include "app_test.h"
#include "uapi/strat_os_net_hw.h"

void APP_test_setup(void)
{
}

void APP_test_teardown(void)
{
}

static boolean APP_net_hw_test_configure(void)
{
    return TRUE;
}

static void test_packet_init_describes_empty_buffer(void)
{
    uint8_t buffer[32];
    STRAT_OS_net_packet_t packet;

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_HW_packet_init(&packet, buffer, sizeof(buffer)));
    TEST_ASSERT_EQUAL_PTR(buffer, packet.data);
    TEST_ASSERT_EQUAL(sizeof(buffer), packet.capacity);
    TEST_ASSERT_EQUAL(0U, packet.offset);
    TEST_ASSERT_EQUAL(0U, packet.length);
}

static void test_complete_rx_descriptor_creates_packet_view(void)
{
    uint8_t buffer[] = { 0x01U, 0x02U, 0x03U, 0x04U };
    STRAT_OS_net_hw_rx_descriptor_t descriptor = {
        buffer,
        sizeof(buffer),
        sizeof(buffer),
        TRUE
    };
    STRAT_OS_net_packet_t packet;

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_HW_packet_from_rx_descriptor(&packet, &descriptor));
    TEST_ASSERT_EQUAL_PTR(buffer, packet.data);
    TEST_ASSERT_EQUAL(sizeof(buffer), packet.capacity);
    TEST_ASSERT_EQUAL(0U, packet.offset);
    TEST_ASSERT_EQUAL(sizeof(buffer), packet.length);
}

static void test_incomplete_rx_descriptor_is_rejected(void)
{
    uint8_t buffer[8];
    STRAT_OS_net_hw_rx_descriptor_t descriptor = {
        buffer,
        sizeof(buffer),
        4U,
        FALSE
    };
    STRAT_OS_net_packet_t packet;

    TEST_ASSERT_EQUAL(ERR_INVALID,
        STRAT_OS_NET_HW_packet_from_rx_descriptor(&packet, &descriptor));
}

static boolean APP_net_hw_test_run(void)
{
    int unity_result;

    APP_test_begin();
    APP_test_run(test_packet_init_describes_empty_buffer);
    APP_test_run(test_complete_rx_descriptor_creates_packet_view);
    APP_test_run(test_incomplete_rx_descriptor_is_rejected);
    unity_result = APP_test_end();
    (void)unity_result;

    return TRUE;
}

boolean APP_sched_configure(void)
{
    return APP_net_hw_test_configure() && APP_net_hw_test_run();
}
