#include "unity.h"
#include "spi_lcl.h"
#include "peripherals/spi.h"
#include "config.h"

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
static void test_configuration(spi_parameter_config_type config_ut);

void setUp(void)
{
    spi_init();

    mock_gpio_intf_init();
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
    spi_parameter_config_type configs[] = {
    /* slck_speed_hz        data_size       bit_order                   endianness              */
    { 100,                    1,              CONFIG_BIT_ORDER_LSB,       CONFIG_ENDIAN_LITTLE   }
    };

    uint8_t i;

    for(i = 0; i < list_cnt(configs); i++)
    {
        resetTest();

        test_configuration(configs[i]);
    }

}

static void test_configuration(spi_parameter_config_type config)
{
    spi_tx_periodic();
}

/* mocked functions */

void delay_us(uint32_t us)
{
    (void)us;
}


/**********************************************************
 * 
 *  TODO
 * 
 * Move all of these functions into a utility file
 *
 */

