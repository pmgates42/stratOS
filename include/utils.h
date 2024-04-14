/**********************************************************
 * 
 *  utils.h
 * 
 * 
 *  DESCRIPTION:
 *      This file contains utility functions
 */

#pragma once

#include "generic.h"

void delay(uint64_t n);
void put32(uint64_t addr, uint32_t val);
uint32_t get32(uint64_t addr);
uint32_t get_el(void);
