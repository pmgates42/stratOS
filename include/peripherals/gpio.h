#pragma once

#include "generic.h"

void gpio_pin_set_func(uint32_t pin, uint8_t fnc);
void gpio_pin_setas_outp(uint32_t pin);
void gpio_pin_setas_inp(uint32_t pin);
void gpio_pin_enable(uint32_t pin);
void gpio_set(uint32_t pin);
void gpio_clr(uint32_t pin);
uint8_t gpio_read_outp(uint32_t pin);