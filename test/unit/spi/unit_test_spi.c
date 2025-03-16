#include "unity.h"
#include "spi_lcl.h"
#include "peripherals/spi.h"
#include "peripherals/gpio.h"
#include "config.h"
#include "mock_gpio.h"

typedef struct
{
    spi_parameter_config_type config;
    uint8_t                   in_bytes[2];
    uint8_t                   out_bytes[2];
} config_test_type;

typedef struct
{
    config_module_id_type module_id;            /* software module id number */
    uint16_t              pin;                  /* physical hardware pin     */
    config_pin_id_type    id;                   /* pin lookup id             */
} consumer_module_pin_config_entry_type;
static const consumer_module_pin_config_entry_type consumer_module_pin_config_table[] =
{
    /*           module_id                pin   id                          */
    /* PIN0 */{  CONFIG_MODULE_ID__SPI,   0,    SPI_MODULE_PIN_ID__CS_0    },
    /* PIN5 */{  CONFIG_MODULE_ID__SPI,   1,    SPI_MODULE_PIN_ID__CS_1    },
    /* PIN1 */{  CONFIG_MODULE_ID__SPI,   2,    SPI_MODULE_PIN_ID__SCLK    },
    /* PIN3 */{  CONFIG_MODULE_ID__SPI,   3,    SPI_MODULE_PIN_ID__MOSI    },
    /* PIN4 */{  CONFIG_MODULE_ID__SPI,   4,    SPI_MODULE_PIN_ID__MISO    },
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
    /* slck_speed_hz        data_size       bit_order                   endianness                    in_bytes        out_bytes */
    { { 100,                    1,              CONFIG_BIT_ORDER_LSB,       CONFIG_ENDIAN_LITTLE   }, { 0xA5, 0xF0 }, { 0xA5, 0xF0 } },
    { { 100,                    1,              CONFIG_BIT_ORDER_MSB,       CONFIG_ENDIAN_LITTLE   }, { 0xA5, 0xF0 }, { 0xA5, 0xF0 } },
    { { 100,                    1,              CONFIG_BIT_ORDER_LSB,       CONFIG_ENDIAN_BIG      }, { 0xA5, 0xF0 }, { 0xA5, 0xF0 } },
    { { 100,                    1,              CONFIG_BIT_ORDER_MSB,       CONFIG_ENDIAN_BIG      }, { 0xA5, 0xF0 }, { 0xA5, 0xF0 } }
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
    mock_gpio_pin_log_type in_log_0;
    mock_gpio_pin_log_type in_log_1;

    setup_config_test(test_vars.config);

    /* TODO remove this */
    /* test the GPIO logger works correctly */
    gpio_set(10);
    gpio_set(10);
    gpio_clr(10);
    gpio_clr(10);

    gpio_clr(11);
    gpio_clr(11);
    gpio_set(11);
    gpio_set(11);

    mock_gpio_get_pin_log(10, MOCK_GPIO_LOG_TYPE__PIN_OUT, &in_log_0);
    mock_gpio_get_pin_log(11, MOCK_GPIO_LOG_TYPE__PIN_OUT, &in_log_1);

    gpio_set(1);

    // spi_tx_periodic();
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
spi_module_error_type     spi_err;

config_register_spi_parameters(spi_params);

/* register SPI parameters */
spi_err = spi_init();
if( SPI_MODULE_ERR__NONE != spi_err )
{
    printf("There was an error initializing the SPI module. Ensure pins and parameters are set correctly. Err: %d", spi_err);
}
}