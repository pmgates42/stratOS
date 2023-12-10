#pragma once

#define STACK_FRAME_SIZE 256

#ifndef __ASSEMBLER__

void vector_init(void);
void vector_enable_irq(void);
void vector_disable_irq(void);

#endif