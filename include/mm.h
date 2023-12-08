/**********************************************************
 * 
 *  mm.h
 * 
 * 
 *  DESCRIPTION:
 *      This file contains memory management definitions.
 *      Among other things, it defines the where the low
 *      low memory is mapped, and the size of the page
 *      table.
 */

#pragma once

#define PAGE_SHIFT	 		    12
#define TABLE_SHIFT 			9
#define SECTION_SHIFT			(PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE   			(1 << PAGE_SHIFT)	
#define SECTION_SIZE			(1 << SECTION_SHIFT)	

#define LOW_MEMORY              (2 * SECTION_SIZE)

#ifndef __ASSEMBLER__

void memzero(unsigned long src, unsigned long n);

#endif
