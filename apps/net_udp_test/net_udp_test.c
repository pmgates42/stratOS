#include "generic.h"
#include "application/app_interface.h"

#include "app_test.h"
#include "uapi/strat_os_net_udp.h"

void APP_test_setup(void)
{
}

void APP_test_teardown(void)
{
}

static boolean APP_net_udp_test_configure(void)
{
    return TRUE;
}

static void test_decode_reads_udp_header_and_payload(void)
{
    uint8_t packet_bytes[] = {
        0x12U, 0x34U, 0x56U, 0x78U, 0x00U, 0x0cU, 0x9aU, 0xbcU,
        0xdeU, 0xadU, 0xbeU, 0xefU
    };
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        sizeof(packet_bytes)
    };
    STRAT_OS_net_udp_datagram_t datagram = { 0 };

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_UDP_decode(&packet, &datagram));
    TEST_ASSERT_EQUAL_UINT16(0x1234U, datagram.source_port);
    TEST_ASSERT_EQUAL_UINT16(0x5678U, datagram.destination_port);
    TEST_ASSERT_EQUAL_UINT16(0x9abcU, datagram.checksum);
    TEST_ASSERT_EQUAL_PTR(packet_bytes + 8U, datagram.payload);
    TEST_ASSERT_EQUAL(4U, datagram.payload_length);
}

static void test_remove_header_exposes_udp_payload(void)
{
    uint8_t packet_bytes[24];
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        16U
    };
    STRAT_OS_net_udp_datagram_t datagram = { 0 };

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        STRAT_OS_NET_UDP_remove_header(&packet, &datagram));
    TEST_ASSERT_EQUAL(8U, packet.offset);
    TEST_ASSERT_EQUAL(8U, packet.length);
}

static void test_short_udp_header_is_rejected(void)
{
    uint8_t packet_bytes[7];
    STRAT_OS_net_packet_t packet = {
        packet_bytes,
        sizeof(packet_bytes),
        0U,
        sizeof(packet_bytes)
    };
    STRAT_OS_net_udp_datagram_t datagram = { 0 };

    TEST_ASSERT_EQUAL(ERR_INVALID,
        STRAT_OS_NET_UDP_decode(&packet, &datagram));
}

static boolean APP_net_udp_test_run(void)
{
    int unity_result;

    APP_test_begin();
    APP_test_run(test_decode_reads_udp_header_and_payload);
    APP_test_run(test_remove_header_exposes_udp_payload);
    APP_test_run(test_short_udp_header_is_rejected);
    unity_result = APP_test_end();
    (void)unity_result;

    return TRUE;
}

boolean APP_sched_configure(void)
{
    return APP_net_udp_test_configure() && APP_net_udp_test_run();
}
