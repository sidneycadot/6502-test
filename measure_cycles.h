
//////////////////////
// measure_cycles.h //
//////////////////////

#ifndef MEASURE_CYCLES_H
#define MEASURE_CYCLES_H

#include <stdint.h>

int16_t measure_cycles(uint8_t * code);
int16_t measure_cycles_zp_safe(uint8_t * code);

void dma_and_interrupts_off(void);
void dma_and_interrupts_on(void);

uint8_t * set_irq_vector_address(uint8_t * newvec);

#define PLATFORM_SPECIFIC_IRQ_OVERHEAD_ATARI  10
#define PLATFORM_SPECIFIC_IRQ_OVERHEAD_C64    10


#define PLATFORM_SPECIFIC_IRQ_OVERHEAD PLATFORM_SPECIFIC_IRQ_OVERHEAD_ATARI

#endif
