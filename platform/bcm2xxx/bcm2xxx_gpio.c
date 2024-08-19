/**********************************************************
 * 
 *  bcm2xxx_gpio.c
 * 
 * 
 *  DESCRIPTION:
 *      GPIO definitions for the raspbery pi BCM2XXX
 *      processors.
 *
 */

#include "generic.h"
#include "bcm2xxx_gpio.h"
#include "utils.h"
#include "peripherals/base.h"
#include "uart.h"
#include "debug.h"

/* Constants */
#define MAX_NMBR_GPIO_PINS 53
#define GPIO_REG_BITS      (sizeof(reg32_t) * 8)

/* Types */

typedef struct
{
    reg32_t reserved;
    uint32_t data[2];

} gpio_pin_data_t;

typedef struct
{
    reg32_t         gpio_fnc_slct[6];   /* GPIO function select registers */
    gpio_pin_data_t output_set;         /* GPIO output set registers */
    gpio_pin_data_t output_clear;       /* GPIO output clear registers */
    gpio_pin_data_t level;              /* GPIO level registers */
    gpio_pin_data_t evnt_detect_sts;    /* GPIO event detect status registers */
    gpio_pin_data_t re_detect;          /* GPIO rising edge detect enable registers */
    gpio_pin_data_t fe_detect;          /* GPIO falling edge detect enable registers */
    gpio_pin_data_t hi_detect;          /* GPIO high detect enable registers */
    gpio_pin_data_t lo_detect;          /* GPIO low detect enable registers */
    gpio_pin_data_t async_re_detect;    /* GPIO async rising edge detect enable registers */
    gpio_pin_data_t async_fe_detect;    /* GPIO async falling edge detect enable registers */
    reg32_t         reserved1;          /* Reserved */
    reg32_t         pupd_enbl;          /* GPIO pull up/down enable registers */
    reg32_t         pupd_enbl_clocks[2];
                                        /* GPIO pull up/down enable clock registers */

} gpio_reg_type;


/* Constants */

#define REG_GPIO_BASE ((volatile gpio_reg_type *)(PBASE + 0x200000))

/**********************************************************
 * 
 *  gpio_pin_set_func
 * 
 * 
 *  DESCRIPTION:
 *      Set the GPIO function for a given pin.
 *
 */
void gpio_pin_set_func(uint32_t pin, uint8_t fnc)
{
/* Local constants */
#define FUNC_SLC_NUM_BITS 3
#define PINS_PER_REG 10

    bcm2xxx_gpio_fnc_t fnc_bcm = (bcm2xxx_gpio_fnc_t)fnc;

    /* Validate input */
    if(pin > MAX_NMBR_GPIO_PINS)
    {
        return;
    }

    /* Find the correct start bit and register index for the
     * provided pin
     */
    uint8_t start_bit = (pin * FUNC_SLC_NUM_BITS) % (FUNC_SLC_NUM_BITS * PINS_PER_REG);
    uint8_t reg_idx = pin / PINS_PER_REG;

    /* Clear the function select register and set the function
     * bits.
     */
    uint32_t fnc_sel_reg = (REG_GPIO_BASE)->gpio_fnc_slct[reg_idx];
    fnc_sel_reg &= ~(7 << start_bit);
    fnc_sel_reg |= (fnc_bcm << start_bit);

    /* Write back to the register */
    (REG_GPIO_BASE)->gpio_fnc_slct[reg_idx] = fnc_sel_reg;

#undef FUNC_SLC_NUM_BITS
#undef PINS_PER_REG
}

/**********************************************************
 * 
 *  gpio_pin_setas_outp
 * 
 * 
 *  DESCRIPTION:
 *      Enable output on the pin
 *
 */
void gpio_pin_setas_outp(uint32_t pin)
{
    gpio_pin_set_func(pin, BCM2XXX_GPIO_FUNC_OUTPUT);
}

/**********************************************************
 * 
 *  gpio_pin_setas_inp
 * 
 * 
 *  DESCRIPTION:
 *      Enable input on the pin
 *
 */
void gpio_pin_setas_inp(uint32_t pin)
{
    gpio_pin_set_func(pin, BCM2XXX_GPIO_FUNC_INPUT);
}

/**********************************************************
 * 
 *  gpio_pin_enable
 * 
 * 
 *  DESCRIPTION:
 *      Enable the pin for GPIO
 *
 */

void gpio_pin_enable(uint32_t pin)
{
    /* Validate input */
    if(pin >= MAX_NMBR_GPIO_PINS)
    {
        return;
    }

    /* Disable pull-up/down */
    REG_GPIO_BASE->pupd_enbl = 0;

    /* Activate clock signal on pin and wait another
     * 150 cycles
     */
    uint8_t idx = pin / GPIO_REG_BITS;

    /* Must delay 150 cycles */
    delay(150);
    REG_GPIO_BASE->pupd_enbl_clocks[idx] = (1 << (pin % GPIO_REG_BITS));
    delay(150);
    REG_GPIO_BASE->pupd_enbl_clocks[idx] = 0;
}

/**********************************************************
 * 
 *  gpio_set
 * 
 * 
 *  DESCRIPTION:
 *      Set the pin
 *
 */

void gpio_set(uint32_t pin)
{
    /* local variables */
    uint8_t data_idx = pin / GPIO_REG_BITS;

    /* Validate input */
    if(pin > MAX_NMBR_GPIO_PINS)
    {
        return;
    }
    REG_GPIO_BASE->output_set.data[data_idx] |= ( 1 << ( pin % GPIO_REG_BITS ) );
}

/**********************************************************
 * 
 *  gpio_clr
 * 
 * 
 *  DESCRIPTION:
 *      Clear the pin
 *
 */

void gpio_clr(uint32_t pin)
{
    /* local variables */
    uint8_t data_idx = pin / GPIO_REG_BITS;

    /* Validate input */
    if(pin > MAX_NMBR_GPIO_PINS)
    {
        return;
    }

    REG_GPIO_BASE->output_clear.data[data_idx] |= ( 1 << ( pin % GPIO_REG_BITS) );
}

/**********************************************************
 * 
 *  gpio_get
 * 
 * 
 *  DESCRIPTION:
 *      Get current pin value
 *
 */

boolean gpio_get(uint32_t pin)
{
    /* local variables */
    uint8_t data_idx = pin / GPIO_REG_BITS;

    /* Validate input */
    if(pin > MAX_NMBR_GPIO_PINS)
    {
        return FALSE;
    }

    return (REG_GPIO_BASE->level.data[data_idx] & (1 << (pin % GPIO_REG_BITS))) ? TRUE : FALSE;
}