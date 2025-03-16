#include "generic.h"
#include "config.h"
#include "mock_gpio.h"

#define TEST_CFG_PIN_CFG_MAX_PIN_NUM  10    /* max number of pins we can keep track of here */

typedef struct
{
    uint32_t    pin;
    uint16_t    out_log_idx;
    uint16_t    in_log_idx;
    mock_gpio_pin_log_type
                out_log;
    mock_gpio_pin_log_type
                in_log;
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
#define SET_BIT(log, bit)  ((log) |= (1U << (bit)))
#define CLR_BIT(log, bit)  ((log) &= ~(1U << (bit)))

    uint8_t i;
    boolean found_entry = FALSE;
    boolean new_log = FALSE;

    /* A log for this pin exists, log the write */
    for(i = 0; i < list_cnt(gpio_test_data); i++)
    {
        if(gpio_test_data[i].pin == pin)
        {
            if(gpio_test_data[i].out_log_idx < (sizeof gpio_test_data[i].out_log * 8))
            {
                found_entry = TRUE;

                if(set)
                    SET_BIT( gpio_test_data[i].out_log, gpio_test_data[i].out_log_idx );
                else
                    CLR_BIT( gpio_test_data[i].out_log, gpio_test_data[i].out_log_idx );

                gpio_test_data[i].out_log_idx++;
            }
            else
            {
                // TODO handle the case where the log for this pin is full
            }
            break;
        }
    }

    /* A log doesn't yet exist for this pin create one now */
    for(i = 0; i < list_cnt(gpio_test_data) && !found_entry; i++)
    {
        /* Find an available slot */
        if(gpio_test_data[i].pin == CFG_INVALID_PIN_NUMBER)
        {
            new_log = TRUE;

            gpio_test_data[i].pin = pin;
            gpio_test_data[i].out_log_idx = 0;
            break;
        }
    }

    /* Recurse the call if we needed to create a new log entry */
    if (!found_entry && new_log && i < list_cnt(gpio_test_data))
        set_gpio_val(set, pin);


#undef SET_BIT
#undef CLR_BIT
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

boolean mock_gpio_get_pin_log(uint32_t pin ,mock_gpio_log_t8 log_type, mock_gpio_pin_log_type * ret_log)
{
    uint8_t i;
    boolean log_vld = FALSE;

    assert( ret_log, "Invalid params" );

    for(i = 0; i < list_cnt(gpio_test_data); i++)
    {
        if(gpio_test_data[i].pin == pin)
        {
            switch(log_type)
            {
                case MOCK_GPIO_LOG_TYPE__PIN_IN:
                    *ret_log = gpio_test_data[i].in_log;
                    log_vld = TRUE;
                    break;

                case MOCK_GPIO_LOG_TYPE__PIN_OUT:
                    *ret_log = gpio_test_data[i].out_log;
                    log_vld - TRUE;
                    break;

                default:
                    log_vld = FALSE;
                    break;
            }

            break;
        }
    }
    return log_vld;
}