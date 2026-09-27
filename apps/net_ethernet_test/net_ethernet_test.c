#include "generic.h"
#include "application/app_interface.h"

#include "app_test.h"
#include "uapi/strat_os_net_ethernet.h"

void APP_test_setup(void)
{
}

void APP_test_teardown(void)
{
}

static boolean APP_net_ethernet_test_configure(void)
{
    return TRUE;
}

static void test_decode_reads_ethernet_header(void)
{
    uint8_t packet_bytes[] = {
        0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U,
        0x20U, 0x21U, 0x22U, 0x23U, 0x24U, 0x25U,
        0x08U, 0x00U,
        0xaaU, 0xbbU
    };
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        sizeof(packet_bytes)
    };
    STRAT_OS_net_ethernet_frame_t frame = { 0 };

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_ETHERNET_decode(&packet, &frame));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(packet_bytes, frame.destination, 6U);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(packet_bytes + 6U, frame.source, 6U);
    TEST_ASSERT_EQUAL_UINT16(STRAT_OS_NET_ETHERNET_TYPE_IPV4, frame.ether_type);
}

static void test_remove_header_exposes_ethernet_payload(void)
{
    uint8_t packet_bytes[32];
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        20U
    };
    STRAT_OS_net_ethernet_frame_t frame = { 0 };

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_ETHERNET_remove_header(&packet, &frame));
    TEST_ASSERT_EQUAL(STRAT_OS_NET_ETHERNET_HEADER_SIZE, packet.offset);
    TEST_ASSERT_EQUAL(6U, packet.length);
}

static void test_short_ethernet_frame_is_rejected(void)
{
    uint8_t packet_bytes[STRAT_OS_NET_ETHERNET_HEADER_SIZE - 1U];
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        sizeof(packet_bytes)
    };
    STRAT_OS_net_ethernet_frame_t frame = { 0 };

    TEST_ASSERT_EQUAL(ERR_INVALID,
        STRAT_OS_NET_ETHERNET_decode(&packet, &frame));
}

static boolean APP_net_ethernet_test_run(void)
{
    int unity_result;

    APP_test_begin();
    APP_test_run(test_decode_reads_ethernet_header);
    APP_test_run(test_remove_header_exposes_ethernet_payload);
    APP_test_run(test_short_ethernet_frame_is_rejected);
    unity_result = APP_test_end();
    (void)unity_result;

    return TRUE;
}

boolean APP_sched_configure(void)
{
    return APP_net_ethernet_test_configure() && APP_net_ethernet_test_run();
}
