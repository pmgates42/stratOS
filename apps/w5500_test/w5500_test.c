/**********************************************************
 *
 *  w5500_test.c
 *
 *  DESCRIPTION:
 *      Unity test app for the W5500 driver.
 *
 *      Host-side tests run against W5500_fake_bus_t on both
 *      platforms. The hardware-gate tests at the bottom talk
 *      to the real chip over the bit-banged SPI pins and are
 *      compiled only for EMBEDDED_BUILD (TEST_IGNORE on
 *      win-sim). Tests run from APP_sched_configure(), i.e.
 *      before boot.c calls spi_init(), so tests that need the
 *      SPI pins call ensure_spi_ready() themselves.
 *
 */

#include "generic.h"
#include "application/app_interface.h"
#include "config.h"
#include "peripherals/spi.h"
#include "debug.h"

#include "app_test.h"
#include "w5500.h"
#include "w5500_fake_bus.h"

static const uint8_t s_test_mac[W5500_MAC_SIZE] = W5500_DEFAULT_MAC;

static W5500_fake_bus_t s_fake;
static w5500_bus_t      s_fake_bus;
static w5500_t          s_dev;

static boolean ensure_spi_ready(void);

void APP_test_setup(void)
{
    clr_mem(&s_dev, sizeof s_dev);
    W5500_FAKE_BUS_init(&s_fake, &s_fake_bus);
    s_dev.bus     = s_fake_bus;
    s_dev.rst_pin = W5500_PIN_NOT_WIRED;
}

void APP_test_teardown(void)
{
}

static boolean APP_w5500_test_configure(void)
{
    return TRUE;
}

/*------------------------------------------------------------
 *  Stage 1: synchronous SPI transfer API
 *------------------------------------------------------------*/

static void test_spi_rejects_chip_select_other_than_zero(void)
{
    TEST_ASSERT_EQUAL(ERR_INVALID, spi_begin(1));
    TEST_ASSERT_EQUAL(ERR_INVALID, spi_end(1));
}

static void test_spi_xfer_outside_transaction_is_rejected(void)
{
    uint8_t byte = 0;

    TEST_ASSERT_EQUAL(ERR_INVALID, spi_xfer(&byte, &byte, 1));
    TEST_ASSERT_EQUAL(ERR_INVALID, spi_end(0));
}

static void test_spi_xfer_accepts_null_tx_and_rx(void)
{
    uint8_t tx[4] = { 0xA5U, 0x5AU, 0xFFU, 0x00U };
    uint8_t rx[4];

    if(!ensure_spi_ready())
    {
        TEST_IGNORE_MESSAGE("spi_init() failed on this platform");
    }

    TEST_ASSERT_EQUAL(ERR_NO_ERR, spi_begin(0));
    TEST_ASSERT_EQUAL(ERR_INVALID, spi_begin(0));           /* already open */
    TEST_ASSERT_EQUAL(ERR_NO_ERR, spi_xfer(tx, rx, sizeof tx));
    TEST_ASSERT_EQUAL(ERR_NO_ERR, spi_xfer(tx, NULL, sizeof tx));
    TEST_ASSERT_EQUAL(ERR_NO_ERR, spi_xfer(NULL, rx, sizeof rx));
    TEST_ASSERT_EQUAL(ERR_NO_ERR, spi_xfer(NULL, NULL, 2));
    TEST_ASSERT_EQUAL(ERR_NO_ERR, spi_xfer(NULL, NULL, 0));
    TEST_ASSERT_EQUAL(ERR_NO_ERR, spi_end(0));
    TEST_ASSERT_EQUAL(ERR_INVALID, spi_end(0));             /* already closed */
}

/*------------------------------------------------------------
 *  Stage 2: register layer against the fake bus
 *------------------------------------------------------------*/

static void test_common_register_read_control_byte(void)
{
    uint8_t value = 0;

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_read8(&s_dev, W5500_BSB_COMMON, W5500_REG_VERSIONR, &value));
    TEST_ASSERT_EQUAL(W5500_VERSIONR_VALUE, value);
    TEST_ASSERT_EQUAL(1, s_fake.log_count);
    TEST_ASSERT_EQUAL_HEX8(0x00, s_fake.log[0].control);
    TEST_ASSERT_EQUAL_HEX16(0x0039, s_fake.log[0].addr);
    TEST_ASSERT_EQUAL(1, s_fake.log[0].len);
    TEST_ASSERT_EQUAL(1, s_fake.begin_calls);
    TEST_ASSERT_EQUAL(1, s_fake.end_calls);
}

static void test_socket_register_write_control_byte(void)
{
    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_write8(&s_dev, W5500_BSB_SOCKET_REG(0), W5500_SN_MR, 0x84U));
    TEST_ASSERT_EQUAL(1, s_fake.log_count);
    TEST_ASSERT_EQUAL_HEX8(0x0C, s_fake.log[0].control);    /* BSB 1, write */
    TEST_ASSERT_EQUAL_HEX16(0x0000, s_fake.log[0].addr);
    TEST_ASSERT_EQUAL_HEX8(0x84, s_fake.sock[0][W5500_SN_MR]);
}

static void test_rx_buffer_read_control_byte(void)
{
    uint8_t data[4] = { 0 };

    s_fake.rx_buf[0x0100] = 0x11U;
    s_fake.rx_buf[0x0101] = 0x22U;
    s_fake.rx_buf[0x0102] = 0x33U;
    s_fake.rx_buf[0x0103] = 0x44U;

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_read(&s_dev, W5500_BSB_SOCKET_RX(0), 0x0100U, data, sizeof data));
    TEST_ASSERT_EQUAL_HEX8(0x18, s_fake.log[0].control);    /* BSB 3, read */
    TEST_ASSERT_EQUAL_HEX16(0x0100, s_fake.log[0].addr);
    TEST_ASSERT_EQUAL(4, s_fake.log[0].len);
    TEST_ASSERT_EQUAL_HEX8(0x11, data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x44, data[3]);
}

static void test_tx_buffer_write_control_byte(void)
{
    uint8_t data[3] = { 0xDEU, 0xADU, 0xBEU };
    uint8_t back[3] = { 0 };

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_write(&s_dev, W5500_BSB_SOCKET_TX(0), 0x3FFEU, data, sizeof data));
    TEST_ASSERT_EQUAL_HEX8(0x14, s_fake.log[0].control);    /* BSB 2, write */
    TEST_ASSERT_EQUAL_HEX16(0x3FFE, s_fake.log[0].addr);
    TEST_ASSERT_EQUAL(3, s_fake.log[0].len);

    /* the fake wraps at 16 KB like the chip does */
    TEST_ASSERT_EQUAL(ERR_NO_ERR, W5500_FAKE_BUS_read_tx(&s_fake, 0x3FFEU, back, sizeof back));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(data, back, sizeof data);
    TEST_ASSERT_EQUAL_HEX8(0xBE, s_fake.tx_buf[0]);
}

static void test_read16_and_write16_are_big_endian(void)
{
    uint16_t value = 0;

    s_fake.common[W5500_REG_SIMR]     = 0x12U;
    s_fake.common[W5500_REG_SIMR + 1] = 0x34U;

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_read16(&s_dev, W5500_BSB_COMMON, W5500_REG_SIMR, &value));
    TEST_ASSERT_EQUAL_HEX16(0x1234, value);

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_write16(&s_dev, W5500_BSB_SOCKET_REG(0), W5500_SN_RX_RD, 0xBEEFU));
    TEST_ASSERT_EQUAL_HEX16(0xBEEF, s_fake.rx_rd);
    TEST_ASSERT_EQUAL(2, s_fake.log[1].len);

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_read16(&s_dev, W5500_BSB_SOCKET_REG(0), W5500_SN_RX_RD, &value));
    TEST_ASSERT_EQUAL_HEX16(0xBEEF, value);
}

static void test_read16_stable_retries_until_two_reads_agree(void)
{
    uint8_t  frame[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    uint16_t value = 0;

    TEST_ASSERT_EQUAL(ERR_NO_ERR, W5500_FAKE_BUS_inject_frame(&s_fake, frame, sizeof frame));
    s_fake.unstable_counter_reads = 2;

    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        w5500_read16_stable(&s_dev, W5500_BSB_SOCKET_REG(0), W5500_SN_RX_RSR, &value));
    TEST_ASSERT_EQUAL(sizeof frame + W5500_RX_LENGTH_PREFIX_SIZE, value);
    /* two perturbed reads, then two agreeing reads */
    TEST_ASSERT_EQUAL(4, s_fake.log_count);
}

static void test_hw_reset_is_noop_when_not_wired(void)
{
    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_hw_reset(&s_dev));
    TEST_ASSERT_EQUAL(0, s_fake.log_count);
}

static void test_sw_reset_clears_mr_rst(void)
{
    s_fake.common[W5500_REG_SIMR] = 0xFFU;

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_sw_reset(&s_dev));
    TEST_ASSERT_EQUAL_HEX8(0x04, s_fake.log[0].control);    /* common write */
    TEST_ASSERT_EQUAL_HEX16(W5500_REG_MR, s_fake.log[0].addr);
    TEST_ASSERT_EQUAL_HEX8(0x00, s_fake.common[W5500_REG_MR]);
    TEST_ASSERT_EQUAL_HEX8(0x00, s_fake.common[W5500_REG_SIMR]);
}

static void test_init_opens_macraw_socket_on_fake(void)
{
    uint8_t sock;

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_init(&s_dev, &s_fake_bus, s_test_mac));
    TEST_ASSERT_TRUE(s_dev.initialized);
    TEST_ASSERT_EQUAL(W5500_PIN_NOT_WIRED, s_dev.rst_pin);

    TEST_ASSERT_EQUAL_HEX8(W5500_SN_SR_MACRAW, s_fake.sock[0][W5500_SN_SR]);
    TEST_ASSERT_EQUAL_HEX8(W5500_SN_MR_MACRAW | W5500_SN_MR_MFEN, s_fake.sock[0][W5500_SN_MR]);
    TEST_ASSERT_EQUAL_HEX8(0x00, s_fake.sock[0][W5500_SN_CR]);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(s_test_mac, &s_fake.common[W5500_REG_SHAR], W5500_MAC_SIZE);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(s_test_mac, s_dev.mac, W5500_MAC_SIZE);

    TEST_ASSERT_EQUAL(16, s_fake.sock[0][W5500_SN_RXBUF_SIZE]);
    TEST_ASSERT_EQUAL(16, s_fake.sock[0][W5500_SN_TXBUF_SIZE]);
    for(sock = 1; sock < W5500_SOCKET_COUNT; sock++)
    {
        TEST_ASSERT_EQUAL(0, s_fake.sock[sock][W5500_SN_RXBUF_SIZE]);
        TEST_ASSERT_EQUAL(0, s_fake.sock[sock][W5500_SN_TXBUF_SIZE]);
    }
}

static void test_init_rejects_wrong_version(void)
{
    s_fake.version = 0x00U;

    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_init(&s_dev, &s_fake_bus, s_test_mac));
    TEST_ASSERT_FALSE(s_dev.initialized);
    TEST_ASSERT_EQUAL_HEX8(W5500_SN_SR_CLOSED, s_fake.sock[0][W5500_SN_SR]);
}

static void test_init_rejects_bus_with_missing_callback(void)
{
    w5500_bus_t bad = s_fake_bus;

    bad.xfer = NULL;
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_init(&s_dev, &bad, s_test_mac));
    TEST_ASSERT_FALSE(s_dev.initialized);
}

static void test_link_up_reads_phycfgr(void)
{
    boolean up = TRUE;

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_link_up(&s_dev, &up));
    TEST_ASSERT_FALSE(up);

    s_fake.common[W5500_REG_PHYCFGR] = W5500_PHYCFGR_LNK;
    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_link_up(&s_dev, &up));
    TEST_ASSERT_TRUE(up);
}

static void test_null_pointers_are_rejected(void)
{
    uint8_t  byte = 0;
    uint16_t word = 0;
    boolean  flag = FALSE;
    w5500_bus_t bus;

    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_bus_from_spi(NULL));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read(NULL, 0, 0, &byte, 1));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read(&s_dev, 0, 0, NULL, 1));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_write(NULL, 0, 0, &byte, 1));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_write(&s_dev, 0, 0, NULL, 1));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read8(NULL, 0, 0, &byte));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read8(&s_dev, 0, 0, NULL));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_write8(NULL, 0, 0, 0));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read16(NULL, 0, 0, &word));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read16(&s_dev, 0, 0, NULL));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_write16(NULL, 0, 0, 0));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read16_stable(NULL, 0, 0, &word));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read16_stable(&s_dev, 0, 0, NULL));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_hw_reset(NULL));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_sw_reset(NULL));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read_version(NULL, &byte));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_read_version(&s_dev, NULL));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_init(NULL, &s_fake_bus, s_test_mac));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_init(&s_dev, NULL, s_test_mac));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_init(&s_dev, &s_fake_bus, NULL));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_link_up(NULL, &flag));
    TEST_ASSERT_EQUAL(ERR_INVALID, w5500_link_up(&s_dev, NULL));

    /* a zero-length transfer with a NULL buffer is allowed */
    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_read(&s_dev, 0, 0, NULL, 0));

    /* nothing above should have touched the bus */
    TEST_ASSERT_EQUAL(1, s_fake.log_count);

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_bus_from_spi(&bus));
    TEST_ASSERT_NOT_NULL(bus.begin);
    TEST_ASSERT_NOT_NULL(bus.xfer);
    TEST_ASSERT_NOT_NULL(bus.end);
}

/*------------------------------------------------------------
 *  Stage 3: hardware gate 1 (real chip over bit-banged SPI)
 *------------------------------------------------------------*/

#ifdef EMBEDDED_BUILD

static w5500_t s_hw_dev;

static boolean prepare_hw_dev(w5500_t *dev)
{
    uint32_t pin;

    if(!ensure_spi_ready())
    {
        return FALSE;
    }

    clr_mem(dev, sizeof *dev);
    if(w5500_bus_from_spi(&dev->bus) != ERR_NO_ERR)
    {
        return FALSE;
    }

    dev->rst_pin = W5500_PIN_NOT_WIRED;
    if(config_lookup_pin(NET_HW_MODULE_PIN_ID__RST, &pin) == CONFIG_ERR_NONE && pin < CFG_INVALID_PIN_NUMBER)
    {
        dev->rst_pin = pin;
    }

    return TRUE;
}

static void test_version_register_reads_0x04(void)
{
    uint8_t version = 0xEEU;

    if(!prepare_hw_dev(&s_hw_dev))
    {
        TEST_FAIL_MESSAGE("SPI not ready");
    }

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_hw_reset(&s_hw_dev));
    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_read_version(&s_hw_dev, &version));

    debug_printf("\n[W5500] VERSIONR = 0x%02x (rst pin %d)", version, s_hw_dev.rst_pin);
    if(version != W5500_VERSIONR_VALUE)
    {
        debug_printf("\n[W5500] 0xFF: MISO stuck high (wiring/CS); 0x00: no power or MISO low; other: clock/phase");
    }
    TEST_ASSERT_EQUAL_HEX8(W5500_VERSIONR_VALUE, version);
}

static void test_init_opens_macraw_socket(void)
{
    uint8_t status = 0xEEU;

    if(!prepare_hw_dev(&s_hw_dev))
    {
        TEST_FAIL_MESSAGE("SPI not ready");
    }

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_init(&s_hw_dev, &s_hw_dev.bus, s_test_mac));
    TEST_ASSERT_TRUE(s_hw_dev.initialized);
    TEST_ASSERT_EQUAL(ERR_NO_ERR,
        w5500_read8(&s_hw_dev, W5500_BSB_SOCKET_REG(0), W5500_SN_SR, &status));
    debug_printf("\n[W5500] Sn_SR(0) = 0x%02x", status);
    TEST_ASSERT_EQUAL_HEX8(W5500_SN_SR_MACRAW, status);
}

static void test_phy_reports_link_when_cable_connected(void)
{
    boolean up = FALSE;

    if(!s_hw_dev.initialized)
    {
        TEST_IGNORE_MESSAGE("device not initialized");
    }

    TEST_ASSERT_EQUAL(ERR_NO_ERR, w5500_link_up(&s_hw_dev, &up));
    debug_printf("\n[W5500] PHY link %s", up ? "UP" : "DOWN");
    if(!up)
    {
        TEST_IGNORE_MESSAGE("link down (cable unplugged?)");
    }
}

#else

static void test_version_register_reads_0x04(void)
{
    TEST_IGNORE_MESSAGE("hardware only");
}

static void test_init_opens_macraw_socket(void)
{
    TEST_IGNORE_MESSAGE("hardware only");
}

static void test_phy_reports_link_when_cable_connected(void)
{
    TEST_IGNORE_MESSAGE("hardware only");
}

#endif /* EMBEDDED_BUILD */

/*------------------------------------------------------------
 *  Runner
 *------------------------------------------------------------*/

static boolean APP_w5500_test_run(void)
{
    int unity_result;

    APP_test_begin();

    APP_test_run(test_spi_rejects_chip_select_other_than_zero);
    APP_test_run(test_spi_xfer_outside_transaction_is_rejected);
    APP_test_run(test_spi_xfer_accepts_null_tx_and_rx);

    APP_test_run(test_common_register_read_control_byte);
    APP_test_run(test_socket_register_write_control_byte);
    APP_test_run(test_rx_buffer_read_control_byte);
    APP_test_run(test_tx_buffer_write_control_byte);
    APP_test_run(test_read16_and_write16_are_big_endian);
    APP_test_run(test_read16_stable_retries_until_two_reads_agree);
    APP_test_run(test_hw_reset_is_noop_when_not_wired);
    APP_test_run(test_sw_reset_clears_mr_rst);
    APP_test_run(test_init_opens_macraw_socket_on_fake);
    APP_test_run(test_init_rejects_wrong_version);
    APP_test_run(test_init_rejects_bus_with_missing_callback);
    APP_test_run(test_link_up_reads_phycfgr);
    APP_test_run(test_null_pointers_are_rejected);

    APP_test_run(test_version_register_reads_0x04);
    APP_test_run(test_init_opens_macraw_socket);
    APP_test_run(test_phy_reports_link_when_cable_connected);

    unity_result = APP_test_end();
    (void)unity_result;

    return TRUE;
}

boolean APP_sched_configure(void)
{
    return APP_w5500_test_configure() && APP_w5500_test_run();
}

/*------------------------------------------------------------
 *  Helpers
 *------------------------------------------------------------*/

/* boot.c runs spi_init() after APP_sched_configure(), so do it here */
static boolean ensure_spi_ready(void)
{
    static boolean attempted;
    static boolean ready;

    if(!attempted)
    {
        attempted = TRUE;
        ready = (spi_init() == ERR_NO_ERR) ? TRUE : FALSE;
    }

    return ready;
}
