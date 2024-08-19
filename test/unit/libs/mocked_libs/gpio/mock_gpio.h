typedef uint64_t mock_gpio_pin_log_type;

typedef uint8_t mock_gpio_log_t8;
enum
{
    MOCK_GPIO_LOG_TYPE__PIN_OUT,
    MOCK_GPIO_LOG_TYPE__PIN_IN,
};

/**********************************************************
 * 
 *  mock_gpio_log_fmt_t8
 * 
 *  DESCRIPTION:
 *     Pin log formats:
 * 
 *     MOCK_GPIO_LOG_FRMT__SHFT_RIGHT
 *          Get the pin log in a shift right format.
 *          e.g., if data was written in the order '0011'
 *                then the log output will be '1100'
 *
 *     MOCK_GPIO_LOG_FRMT__SHFT_LEFT
 *          Get the pin log in a shift right format.
 *          e.g., if data was written in the order '0011'
 *                then the log output will be '0011'
 */

typedef uint8_t mock_gpio_log_fmt_t8;
enum
{
    MOCK_GPIO_LOG_FRMT__SHFT_RIGHT,
    MOCK_GPIO_LOG_FRMT__SHFT_LEFT,
};

/**********************************************************
 * 
 *  mock_gpio_intf_reset_logs()
 * 
 *  DESCRIPTION:
 *     Initialize this interface.
 *     
 */

void mock_gpio_intf_init();

/**********************************************************
 * 
 *  mock_gpio_get_pin_log()
 * 
 *  DESCRIPTION:
 *     Get pin log (out or in).  Returns the "raw" format
 *     that the GPIO pin logger stores pin writes.
 * 
 */

boolean mock_gpio_get_pin_log(uint32_t pin ,mock_gpio_log_t8 log_type, mock_gpio_pin_log_type * ret_log);

/**********************************************************
 * 
 *  mock_gpio_get_pin_log_fmt()
 * 
 *  DESCRIPTION:
 *     Get pin log (out or in) in a given format
 *     (mock_gpio_log_fmt_t8)
 *     
 */

boolean mock_gpio_get_pin_log_fmt(uint32_t pin, mock_gpio_log_t8 log_type, mock_gpio_pin_log_type * ret_log, mock_gpio_log_fmt_t8 frmt);


/**********************************************************
 * 
 *  mock_gpio_intf_reset_logs()
 * 
 *  DESCRIPTION:
 *     Clear all of the logs
 *     
 */

void mock_gpio_intf_reset_logs();
