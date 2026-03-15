/*
 *  strat_os_gpio.h
 *
 *  StratOS Consumer GPIO interface
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef char STRAT_OS_GPIO_err_type;
enum
{
    STRAT_OS_GPIO_ERR__NO_ERROR = 0
};

typedef char STRAT_OS_GPIO_config_module_flags;
enum
{
    STRAT_OS_GPIO_MDL_ERR_LVL__NONE_BIT           = 0,
    STRAT_OS_GPIO_MDL_ERR_LVL__WARN_PIN_CLASH_BIT = 1,
    STRAT_OS_GPIO_MDL_ERR_LVL__ERR_PIN_CLASH_BIT  = 2,
    STRAT_OS_GPIO_MDL_ERR_LVL__WARN_PIN_UNCONFIGURED_BIT = 3,
    STRAT_OS_GPIO_MDL_ERR_LVL__ERR_PIN_UNCONFIGURED_BIT  = 2,
};

STRAT_OS_GPIO_err_type STRAT_OS_gpio_set_module_pin_config_error_flags(unsigned int pin);
STRAT_OS_GPIO_err_type STRAT_OS_gpio_set(unsigned int pin);
STRAT_OS_GPIO_err_type STRAT_OS_gpio_clear(unsigned int pin);
STRAT_OS_GPIO_err_type STRAT_OS_gpio_enable_pin(unsigned int pin);
STRAT_OS_GPIO_err_type STRAT_OS_gpio_pin_setas_input(unsigned int pin);
STRAT_OS_GPIO_err_type STRAT_OS_gpio_pin_setas_output(unsigned int pin);

#ifdef __cplusplus
}
#endif
