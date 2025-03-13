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

#ifndef EMBEDDED_BUILD
#include <string.h>
#include <stdio.h>
#endif

#ifdef EMBEDDED_BUILD
#define NULL  0
#endif

#define TRUE  1
#define FALSE 0

#define BIT(n) (1UL << (n))

#define BS_ALL ( 0 - 1 )
#define BS_ALL_32 BS_ALL
#define BS_MAX(size) ( BS_ALL & ~( 1 << ((size * 8) -1) ) )

#define assert(condition, msg) \

typedef int sint32_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;
typedef volatile uint32_t reg32_t;
typedef char boolean;
typedef unsigned long size_t;

typedef void ( *void_func_t )( void );

#define clr_mem(ptr, size) do { \
    unsigned char *clr_mem_ptr = (unsigned char *)(ptr); \
    size_t clr_mem_size = (size); \
    while (clr_mem_size--) *clr_mem_ptr++ = 0; \
} while(0)

#define list_cnt(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef EMBEDDED_BUILD
void memcpy(void *dest, const void *src, size_t n);
#endif

static inline uint8_t get_bit(uint32_t value, uint8_t bit) {
    return (value >> bit) & 1U;
}
