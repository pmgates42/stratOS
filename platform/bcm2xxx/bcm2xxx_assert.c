/**********************************************************
 * 
 *  bcm2xxx_assert.c
 * 
 *  DESCRIPTION:
 *      Implements bcm2xxx specific assert functionality
 */

#include "printf.h"

/**********************************************************
 * 
 *  PLATFORM_assert_failed()
 * 
 *  DESCRIPTION:
 *      Called when an assertion fails
 * 
 */

 void PLATFORM_assert_failed(const char * msg)
 {
     printf(msg);
    //  exit(1); // TODO
 }
