/**********************************************************
 * 
 *  generic.h
 * 
 * 
 *  DESCRIPTION:
 *      This file contains generic types and definitions
 *
 */

#pragma once

#define NULL  0

#define TRUE  1
#define FALSE 0

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;
typedef volatile uint32_t reg32_t;
typedef char boolean;

typedef void ( *void_func_t )( void );

int clr_mem( void *mem, uint32_t size );
int list_cnt( void *list );
