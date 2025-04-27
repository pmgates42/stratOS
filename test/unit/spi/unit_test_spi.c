#include "unity.h"
#include "spi_lcl.h"
#include "peripherals/spi.h"
#include "peripherals/gpio.h"
#include "config.h"
#include "mock_gpio.h"

enum
{
    TEST_PHYS_PIN__CS0 = 0,
    TEST_PHYS_PIN__CS1,
    TEST_PHYS_PIN__SCLK,
    TEST_PHYS_PIN__MOSI,
    TEST_PHYS_PIN__MISO,
};

typedef struct
{
    spi_parameter_config_type config;
    uint64_t                  in_bytes;
    uint64_t                  out_bytes;
} config_test_type;

typedef struct
{
    config_module_id_type module_id;            /* software module id number */
    uint16_t              pin;                  /* physical hardware pin     */
    config_pin_id_type    id;                   /* pin lookup id             */
} consumer_module_pin_config_entry_type;
static const consumer_module_pin_config_entry_type consumer_module_pin_config_table[] =
{
    /*           module_id                pin                    id                          */
    /* PIN0 */{  CONFIG_MODULE_ID__SPI,   TEST_PHYS_PIN__CS0,    SPI_MODULE_PIN_ID__CS_0    },
    /* PIN5 */{  CONFIG_MODULE_ID__SPI,   TEST_PHYS_PIN__CS1,    SPI_MODULE_PIN_ID__CS_1    },
    /* PIN1 */{  CONFIG_MODULE_ID__SPI,   TEST_PHYS_PIN__SCLK,   SPI_MODULE_PIN_ID__SCLK    },
    /* PIN3 */{  CONFIG_MODULE_ID__SPI,   TEST_PHYS_PIN__MOSI,   SPI_MODULE_PIN_ID__MOSI    },
    /* PIN4 */{  CONFIG_MODULE_ID__SPI,   TEST_PHYS_PIN__MISO,   SPI_MODULE_PIN_ID__MISO    },
};

static void test_configurations();
static void test_configuration(config_test_type config);
static boolean register_module_pins(void);
static void setup_drivers(spi_parameter_config_type spi_params);

void setUp()
{

}

void setup_config_test(spi_parameter_config_type spi_params)
{
    /* These are in order: 
        1. Initialize config module 
        2. Register testing pins with the config module 
        3. Initialize the mocked GPIO interface
        4. Initialize the SPI driver */
    config_module_init();
    register_module_pins();
    mock_gpio_intf_init();
    setup_drivers(spi_params);
}

void tearDown(void)
{
}

void resetTest(void)
{

}

int main() {

    test_configurations();

    return 0;
}

static void test_configurations()
{
    config_test_type test_vars[] = {
    /* slck_speed_hz        data_size           bit_order                   endianness                in_bytes      out_bytes */
    // { { 100,                sizeof(uint16_t),   CONFIG_BIT_ORDER_LSB,       CONFIG_ENDIAN_LITTLE   }, 0xA5F0,       0x0FA5 },
    // { { 100,                sizeof(uint16_t),   CONFIG_BIT_ORDER_MSB,       CONFIG_ENDIAN_LITTLE   }, 0xA5F0,       0xF0A5 },
    // { { 100,                sizeof(uint16_t),   CONFIG_BIT_ORDER_LSB,       CONFIG_ENDIAN_BIG      }, 0xA5F0,       0xF0A5 },
    { { 100,                sizeof(uint16_t),   CONFIG_BIT_ORDER_MSB,       CONFIG_ENDIAN_BIG      }, 0xA5F0,          0x0FA5 }
    };

    uint8_t i;

    for(i = 0; i < list_cnt(test_vars); i++)
    {
        resetTest();

        test_configuration(test_vars[i]);
    }

}

static void test_configuration(config_test_type test_vars)
{
    mock_gpio_pin_log_type mosi_pin_log;
    boolean                mosi_log_vld;
    hw_intf_data_t         hw_intf_data;

    clr_mem(&mosi_pin_log, sizeof mosi_pin_log);
    clr_mem(&hw_intf_data, sizeof hw_intf_data);

    /* Set SPI config for this test */
    setup_config_test(test_vars.config);

    /* Read out the config and verify pins were set correctly */
    spi_lcl_get_hw_intf_data(&hw_intf_data);
    TEST_ASSERT_EQUAL_UINT32(TEST_PHYS_PIN__CS0,    hw_intf_data.gpio_pins.cs_0);
    TEST_ASSERT_EQUAL_UINT32(TEST_PHYS_PIN__CS1,    hw_intf_data.gpio_pins.cs_1);
    TEST_ASSERT_EQUAL_UINT32(TEST_PHYS_PIN__SCLK,   hw_intf_data.gpio_pins.sclk);
    TEST_ASSERT_EQUAL_UINT32(TEST_PHYS_PIN__MOSI,   hw_intf_data.gpio_pins.mosi);
    TEST_ASSERT_EQUAL_UINT32(TEST_PHYS_PIN__MISO,   hw_intf_data.gpio_pins.miso);

    /* Call testing function */
    spi_tx_periodic();

    /* Verify TX'd data */
    mosi_log_vld = mock_gpio_get_pin_log_fmt(hw_intf_data.gpio_pins.mosi, MOCK_GPIO_LOG_TYPE__PIN_OUT,
                                            &mosi_pin_log, MOCK_GPIO_LOG_FRMT__SHFT_LEFT);

    TEST_ASSERT_EQUAL_UINT8(TRUE, mosi_log_vld);
    TEST_ASSERT_EQUAL_UINT64(test_vars.out_bytes, mosi_pin_log);
    
}

/* mocked functions */

void delay_us(uint32_t us)
{
    (void)us;
}

static boolean register_module_pins(void)
{
uint8_t       i;
config_err_t8 config_err;

for(i = 0; i < list_cnt( consumer_module_pin_config_table ); i++ )
{
    config_err = config_register_pin_for_module( consumer_module_pin_config_table[i].module_id,
                                                  consumer_module_pin_config_table[i].pin,
                                                  consumer_module_pin_config_table[i].id );

    if( config_err != CONFIG_ERR_NONE )
        {
        return FALSE;
        }
}

return TRUE;
}

static void setup_drivers(spi_parameter_config_type spi_params)
{
error_type     spi_err;

config_register_spi_parameters(spi_params);

/* register SPI parameters */
spi_err = spi_init();
if( ERR_NO_ERR != spi_err )
{
    printf("There was an error initializing the SPI module. Ensure pins and parameters are set correctly. Err: %d", spi_err);
}
}