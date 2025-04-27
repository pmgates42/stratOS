/**********************************************************
 * 
 *  platform_assert.h
 * 
 *  DESCRIPTION:
 *     Contains the interface for platform specific
 *      assertion behavior.
 */

#pragma once

/**********************************************************
 * 
 *  PLATFORM_assert_failed()
 * 
 *  DESCRIPTION:
 *      Called when an assertion fails
 * 
 *  REQUIRED:
 *      Yes.
 */

void PLATFORM_assert_failed(const char * msg);
