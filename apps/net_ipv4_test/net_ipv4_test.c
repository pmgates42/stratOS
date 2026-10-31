#include "generic.h"
#include "application/app_interface.h"

#include "app_test.h"
#include "uapi/strat_os_net_ipv4.h"

void APP_test_setup(void)
{
}

void APP_test_teardown(void)
{
}

static boolean APP_net_ipv4_test_configure(void)
{
    return TRUE;
}

static void test_decode_reads_non_fragmented_ipv4_header(void)
{
    uint8_t packet_bytes[] = {
        0x45U, 0x00U, 0x00U, 0x1cU, 0x12U, 0x34U, 0x40U, 0x00U,
        0x40U, 0x11U, 0xa5U, 0x47U, 0xc0U, 0xa8U, 0x01U, 0x02U,
        0xc0U, 0xa8U, 0x01U, 0x03U, 0xdeU, 0xadU, 0xbeU, 0xefU,
        0x00U, 0x01U, 0x02U, 0x03U
    };
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        sizeof(packet_bytes)
    };
    STRAT_OS_net_ipv4_packet_t ipv4 = { 0 };

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_IPV4_decode(&packet, &ipv4));
    TEST_ASSERT_EQUAL_UINT8(4U, ipv4.version);
    TEST_ASSERT_EQUAL_UINT8(20U, ipv4.header_length);
    TEST_ASSERT_EQUAL_UINT8(STRAT_OS_NET_IPV4_PROTOCOL_UDP, ipv4.protocol);
    TEST_ASSERT_EQUAL_UINT8(64U, ipv4.time_to_live);
    TEST_ASSERT_EQUAL_UINT16(28U, ipv4.total_length);
    TEST_ASSERT_EQUAL_UINT16(0x1234U, ipv4.identification);
    TEST_ASSERT_EQUAL_UINT16(0x4000U, ipv4.flags_and_fragment_offset);
    TEST_ASSERT_EQUAL_UINT16(0xa547U, ipv4.header_checksum);
    TEST_ASSERT_EQUAL_UINT32(0xc0a80102UL, ipv4.source_address);
    TEST_ASSERT_EQUAL_UINT32(0xc0a80103UL, ipv4.destination_address);
}

static void test_remove_header_exposes_ipv4_payload(void)
{
    uint8_t packet_bytes[28];
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        sizeof(packet_bytes)
    };
    STRAT_OS_net_ipv4_packet_t ipv4 = {
        4U,
        20U,
        STRAT_OS_NET_IPV4_PROTOCOL_UDP,
        64U,
        28U,
        0U,
        0U,
        0U,
        0U,
        0U
    };

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_IPV4_remove_header(&packet, &ipv4));
    TEST_ASSERT_EQUAL(20U, packet.offset);
    TEST_ASSERT_EQUAL(8U, packet.length);
}

static void test_short_ipv4_header_is_rejected(void)
{
    uint8_t packet_bytes[STRAT_OS_NET_IPV4_MIN_HEADER_SIZE - 1U];
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        sizeof(packet_bytes)
    };
    STRAT_OS_net_ipv4_packet_t ipv4 = { 0 };

    TEST_ASSERT_EQUAL(ERR_INVALID,
        STRAT_OS_NET_IPV4_decode(&packet, &ipv4));
}

static void test_ipv4_decode_rejects_fragmented_packet_for_now(void)
{
    uint8_t packet_bytes[STRAT_OS_NET_IPV4_MIN_HEADER_SIZE] = { 0 };
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        sizeof(packet_bytes)
    };
    STRAT_OS_net_ipv4_packet_t ipv4 = { 0 };

    packet_bytes[0] = 0x45U;
    packet_bytes[6] = 0x20U;
    packet_bytes[7] = 0x00U;

    TEST_ASSERT_EQUAL(ERR_INVALID,
        STRAT_OS_NET_IPV4_decode(&packet, &ipv4));
}

static boolean APP_net_ipv4_test_run(void)
{
    int unity_result;

    APP_test_begin();
    APP_test_run(test_decode_reads_non_fragmented_ipv4_header);
    APP_test_run(test_remove_header_exposes_ipv4_payload);
    APP_test_run(test_short_ipv4_header_is_rejected);
    APP_test_run(test_ipv4_decode_rejects_fragmented_packet_for_now);
    unity_result = APP_test_end();
    (void)unity_result;

    return TRUE;
}

boolean APP_sched_configure(void)
{
    return APP_net_ipv4_test_configure() && APP_net_ipv4_test_run();
}
