/**********************************************************
 * 
 *  sim_assert.c
 * 
 *  DESCRIPTION:
 *      Implements sim specific assert functionality
 */


#include <stdlib.h>
#include <stdio.h>
#include "generic.h"

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
    exit(1);
}