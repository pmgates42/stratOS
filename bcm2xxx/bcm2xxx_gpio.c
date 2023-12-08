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
#include "bcm2xxx_pvg_gpio.h"
#include "utils.h"
#include "peripherals/base.h"

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

} bcm2xxx_gpio_reg_t;


/* Constants */

#define REG_GPIO_BASE ((volatile bcm2xxx_gpio_reg_t *)(PBASE + 0x200000))

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
 *  gpio_pin_enable
 * 
 * 
 *  DESCRIPTION:
 *      Enable the pin for GPIO
 *
 */

void gpio_pin_enable(uint32_t pin)
{
    /* Disable pull-up/down */
    REG_GPIO_BASE->pupd_enbl = 0;

    /* Must delay 150 cycles */
    delay(150);

    /* Activate clock signal on pin and wait another
     * 150 cycles
     */
    uint8_t reg_size_bits = sizeof(reg32_t) * 8;
    uint8_t idx = pin / reg_size_bits;
    REG_GPIO_BASE->pupd_enbl_clocks[idx] = (1 << (pin % reg_size_bits));
    delay(150);
 
    /* Disable pull-up/down */
    REG_GPIO_BASE->pupd_enbl = 0;
    REG_GPIO_BASE->pupd_enbl_clocks[idx] = 0;
}