#pragma once

#include "generic.h"

/**********************************************************
 * 
 *  spi_mode_type
 * 
 *  DESCRIPTION:
 *     Type of device we are (master/slave)
 */

typedef enum
{
    SPI_OPERATING_MODE__MASTER,
    SPI_OPERATING_MODE__SLAVE,
}   spi_mode_type;

/*
cs_mode_type tells the high-level (logical) driver code what
chip select behavior it should use.

CHANNEL_OPERATING_MODE__STANDARD
assumes standard behavior, CS lines exist for every device. When
the pin goes low, we are TXing, when the pin goes high, we are
done TXing.

CHANNEL_OPERATING_MODE__COMBINATORIAL will use a combinatorial
approach where we make the assumption that all devices have
knowledge of all CS pins in the system. This means we can combine
them to create additional channels allowing more devices on the bus.

For example:

2 Chip Selects means it is technically possible to support
3 channels:

CS0	CS1	Selected Device
0	1	Device A
1	0	Device B
0	0	Device C
1	1	None
*/

typedef enum
{
    CHANNEL_OPERATING_MODE__STANDARD,      /* default */
    CHANNEL_OPERATING_MODE__COMBINATORIAL, /* (not implemented) */
} cs_mode_type;

/**********************************************************
 * 
 *  PLATFORM_spi_init()
 * 
 *  DESCRIPTION:
 *     Do platform specific SPI driver initialization. 
 * 
 *  REQUIRED:
 *      The expectation is that the platform will register
 *      the following OS pins with the config module for use
 *      in the high level (logical) SPI driver code:
 * 
 *      SPI_MODULE_PIN_ID__CS_0
 *      SPI_MODULE_PIN_ID__CS_1 (optional)
 *      SPI_MODULE_PIN_ID__SCLK
 *      SPI_MODULE_PIN_ID__MOSI
 *      SPI_MODULE_PIN_ID__MISO
 * 
 *  RETURNS:
 *      Number of chip selects supported. Zero will be an
 *      indication that an error has occured. More than 2
 *      will be considered an invalid state.
 */

uint8_t PLATFORM_spi_init(void);

/**********************************************************
 * 
 *  PLATFORM_spi_get_operating_mode()
 * 
 *  DESCRIPTION:
 *     Get operating mode.
 */

spi_mode_type PLATFORM_spi_get_operating_mode(void);

/**********************************************************
 * 
 *  PLATFORM_spi_get_cs_operating_mode()
 * 
 *  DESCRIPTION:
 *     Get the SPI (CS) operating mode from the implementing
 *     platform.
 */

cs_mode_type PLATFORM_spi_get_cs_operating_mode(void);
