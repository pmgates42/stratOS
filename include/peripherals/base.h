#pragma once

// todo move this into rpi specific file
/* Set the correct Peripheral base */

#if RPI_VERSION == 3
#define PBASE  0x3F000000UL

#elif RPI_VERSION == 4
#define PBASE  0xFE000000UL

#else
#define PBASE  0x00000000UL
#error "Unknown PI version"

#endif
