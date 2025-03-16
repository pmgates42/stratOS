typedef uint64_t mock_gpio_pin_log_type;

typedef uint8_t mock_gpio_log_t8;
enum
{
    MOCK_GPIO_LOG_TYPE__PIN_OUT,
    MOCK_GPIO_LOG_TYPE__PIN_IN,
};

void mock_gpio_intf_init();
boolean mock_gpio_get_pin_log(uint32_t pin ,mock_gpio_log_t8 log_type, mock_gpio_pin_log_type * ret_log);
