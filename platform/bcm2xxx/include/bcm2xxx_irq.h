/**********************************************************
 * 
 *  bcm2xxx_irq.h
 * 
 * 
 *  DESCRIPTION:
 *      BCM2xxx IRQ interface
 *
 */

#pragma once

#include "generic.h"

typedef uint8_t bcm2xxx_irq_periph_t8;  /* ARM peripheral interrupts table type */
enum
    {
    BCM2XXX_IRQ_PERIPH_SYS_TMR_M1    = 1,  /* System timer match 1 */
    BCM2XXX_IRQ_PERIPH_SYS_TMR_M2    = 3,  /* System timer match 3 */
    BCM2XXX_IRQ_PERIPH_USB_CTRL      = 9,  /* USB Controller       */
    BCM2XXX_IRQ_PERIPH_AUX_INT       = 20, /* Auxillary peripherals*/
    BCM2XXX_IRQ_PERIPH_I2C_SPI_SLAVE = 43, /* i2c/spi slave        */
    BCM2XXX_IRQ_PERIPH_PWA_0         = 45,
    BCM2XXX_IRQ_PERIPH_PWA_1         = 46,
    BCM2XXX_IRQ_PERIPH_SMI           = 48,
    BCM2XXX_IRQ_PERIPH_GPIO_0        = 49,
    BCM2XXX_IRQ_PERIPH_GPIO_1        = 50,
    BCM2XXX_IRQ_PERIPH_GPIO_2        = 51,
    BCM2XXX_IRQ_PERIPH_GPIO_3        = 52, 
    BCM2XXX_IRQ_PERIPH_I2C           = 53,
    BCM2XXX_IRQ_PERIPH_SPI           = 54,
    BCM2XXX_IRQ_PERIPH_PCM           = 55,
    BCM2XXX_IRQ_PERIPH_UART          = 57,
    BCM2XXX_IRQ_PERIPH_COUNT
    };
