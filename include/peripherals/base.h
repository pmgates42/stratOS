#pragma once

/* Set the correct Peripheral base 
 * or physical base of the rpi's MMIO.
 */

#if RPI_VERSION == 3
#define PBASE  0x3F000000UL

#elif RPI_VERSION == 4
#define PBASE  0xFE000000UL

#else
#define PBASE  0x00000000UL
#error "Unknown PI version"

#endif
