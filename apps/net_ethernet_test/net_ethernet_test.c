#include "generic.h"
#include "application/app_interface.h"

#include "app_test.h"
#include "uapi/strat_os_net_ethernet.h"

#define TEST_ETHERNET_PAYLOAD_SIZE 2U

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
        /* Destination MAC address. */
        0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U,
        /* Source MAC address. */
        0x20U, 0x21U, 0x22U, 0x23U, 0x24U, 0x25U,
        /* EtherType: IPv4. */
        0x08U, 0x00U,
        /* Payload bytes, not interpreted by the Ethernet layer. */
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
    TEST_ASSERT_EQUAL_UINT8_ARRAY(
        packet_bytes + STRAT_OS_NET_ETHERNET_DESTINATION_OFFSET,
        frame.destination,
        STRAT_OS_NET_ETHERNET_ADDRESS_SIZE);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(
        packet_bytes + STRAT_OS_NET_ETHERNET_SOURCE_OFFSET,
        frame.source,
        STRAT_OS_NET_ETHERNET_ADDRESS_SIZE);
    TEST_ASSERT_EQUAL_UINT16(STRAT_OS_NET_ETHERNET_TYPE_IPV4, frame.ether_type);
}

static void test_remove_header_exposes_ethernet_payload(void)
{
    /* Ethernet header followed by bytes owned by the next protocol layer. */
    uint8_t packet_bytes[32];
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        STRAT_OS_NET_ETHERNET_HEADER_SIZE + TEST_ETHERNET_PAYLOAD_SIZE
    };
    STRAT_OS_net_ethernet_frame_t frame = { 0 };

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_ETHERNET_remove_header(&packet, &frame));
    TEST_ASSERT_EQUAL(STRAT_OS_NET_ETHERNET_HEADER_SIZE, packet.offset);
    TEST_ASSERT_EQUAL(TEST_ETHERNET_PAYLOAD_SIZE, packet.length);
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
