/*
 *  strat_os_gpio.h
 *
 *  StratOS Consumer GPIO interface
 *
 *  Copyright (c) 2025 Paul M. Gates
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef STRAT_OS_GPIO_H
#define STRAT_OS_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************
 * 
 *  STRAT_OS_GPIO_err_type
 * 
 * 
 *  DESCRIPTION:
 *      Error types returned by the functions declared here
 *
 */

typedef char STRAT_OS_GPIO_err_type;
enum
{
    STRAT_OS_GPIO_ERR__NO_ERROR = 0
};

/**********************************************************
 * 
 *  STRAT_OS_GPIO_config_module_flags
 * 
 * 
 *  DESCRIPTION:
 *      Config module flags. These are used to tell
 *      StratOS what kind of error handling is required for
 *      pin config issues.
 * 
 *      Example, if the desired behavior of the system is to
 *      return an error when writing to an unconfigured pin,
 *      then set STRAT_OS_GPIO_MDL_ERR_LVL__WARN_PIN_UNCONFIGURED_BIT
 *
 *      For use in STRAT_OS_gpio_set_module_pin_config_error_flags()
 *
 */

typedef char STRAT_OS_GPIO_config_module_flags;
enum
{
    STRAT_OS_GPIO_MDL_ERR_LVL__NONE_BIT           = 0,         /* Don't error on config issues          */

    /*
     * Warn or Error on reads/writes to GPIO pins that belong to another module
     */
    STRAT_OS_GPIO_MDL_ERR_LVL__WARN_PIN_CLASH_BIT = 1,
    STRAT_OS_GPIO_MDL_ERR_LVL__ERR_PIN_CLASH_BIT  = 2,

    /*
     * Warn or Error on reads/writes to GPIO pins that are not configured by the
     * module that is doing the read/write
     */
    STRAT_OS_GPIO_MDL_ERR_LVL__WARN_PIN_UNCONFIGURED_BIT = 3,
    STRAT_OS_GPIO_MDL_ERR_LVL__ERR_PIN_UNCONFIGURED_BIT  = 2,
};

/**********************************************************
 * 
 *  STRAT_OS_gpio_set
 * 
 * 
 *  DESCRIPTION:
 *      Set a GPIO pin
 *
 */

STRAT_OS_GPIO_err_type STRAT_OS_gpio_set_module_pin_config_error_flags(unsigned int pin);

/**********************************************************
 * 
 *  STRAT_OS_gpio_set
 * 
 * 
 *  DESCRIPTION:
 *      Set a GPIO pin
 *
 */

STRAT_OS_GPIO_err_type STRAT_OS_gpio_set(unsigned int pin);

/**********************************************************
 * 
 *  STRAT_OS_gpio_set
 * 
 * 
 *  DESCRIPTION:
 *      Clear a GPIO pin
 *
 */

STRAT_OS_GPIO_err_type STRAT_OS_gpio_clear(unsigned int pin);

/**********************************************************
 * 
 *  STRAT_OS_gpio_enable_pin
 * 
 * 
 *  DESCRIPTION:
 *      Enable a GPIO pin
 *
 */

STRAT_OS_GPIO_err_type STRAT_OS_gpio_enable_pin(unsigned int pin);

/**********************************************************
 * 
 *  STRAT_OS_gpio_pin_setas_input
 * 
 * 
 *  DESCRIPTION:
 *      Set GPIO pin as INPUT
 *
 */

STRAT_OS_GPIO_err_type STRAT_OS_gpio_pin_setas_input(unsigned int pin);


/**********************************************************
 * 
 *  STRAT_OS_gpio_pin_setas_output
 * 
 * 
 *  DESCRIPTION:
 *      Set GPIO pin as OUTPUT
 *
 */

STRAT_OS_GPIO_err_type STRAT_OS_gpio_pin_setas_output(unsigned int pin);

#ifdef __cplusplus
}
#endif

#endif /* STRAT_OS_GPIO_H */
