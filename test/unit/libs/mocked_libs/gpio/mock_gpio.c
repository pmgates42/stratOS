#include "generic.h"
#include "config.h"

#define GPIO_LOG_BUFFER_MAX_SZ  250         /* number of elements in each log (out and in) */
#define TEST_CFG_PIN_CFG_MAX_PIN_NUM  10    /* max number of pins we can keep track of here */

typedef struct
{
    uint32_t    pin;
    uint16_t    out_log_idx;
    uint16_t    in_log_idx;
    boolean     out_log[GPIO_LOG_BUFFER_MAX_SZ];
    boolean     in_log[GPIO_LOG_BUFFER_MAX_SZ];
} gpio_test_data_type;

static gpio_test_data_type gpio_test_data[TEST_CFG_PIN_CFG_MAX_PIN_NUM];

static void set_gpio_val(boolean set, uint32_t pin);

void gpio_set(uint32_t pin)
{
    set_gpio_val(TRUE, pin);
}

void gpio_clr(uint32_t pin)
{
    set_gpio_val(FALSE, pin);
}

static void set_gpio_val(boolean set, uint32_t pin)
{
    uint8_t i;

    for(i = 0; i < list_cnt(gpio_test_data); i++)
    {
        if(gpio_test_data[i].pin == pin)
        {
            if(gpio_test_data[i].in_log_idx < GPIO_LOG_BUFFER_MAX_SZ)
            {
                gpio_test_data[i].in_log[gpio_test_data[i].in_log_idx] = set;
                gpio_test_data[i].in_log_idx++;
            }
        }
    }
}

void mock_gpio_intf_init()
{
uint16_t i;

clr_mem(gpio_test_data, sizeof gpio_test_data);

for(i = 0; i < list_cnt(gpio_test_data); i++)
{
    gpio_test_data[i].pin = CFG_INVALID_PIN_NUMBER;
}

}

