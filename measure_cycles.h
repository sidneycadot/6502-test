
//////////////////////
// measure_cycles.h //
//////////////////////

#ifndef MEASURE_CYCLES_H
#define MEASURE_CYCLES_H

#include <stdint.h>

// Enable/disable DMA and interrupts, to create a situation where the 6502 timing behaves in a way that
// allows the 'measure_cycles' and 'measure_cycles_zp_safe' to do their job.

void dma_and_interrupts_off(void);
void dma_and_interrupts_on(void);

// Measure the number of cycles that a code fragment will take, up to (but not including) a final RTS.
//
// The 'measure_cycles_zp_safe' version saves page 0 (addresses 0x00 .. 0xff) before the measurement and
// restores it afterward; that version of the routine should be used if the test will write to page 0.

int16_t measure_cycles(uint8_t * code);
int16_t measure_cycles_zp_safe(uint8_t * code);

// Code and constants to implement the BRK timing test.
//
// The BRK instruction will vector through the IRQ vector at (0xfffe, 0xffff).
// Depending on the platform, this may take the CPU into an OS service routine.
// It is assumed that, at some point, and after a predictable number of cycles,
// it will be possible to route execution to a routine under user control, by
// altering some vector address in memory.
//
// The 'set_irq_vector_address' will change this vector address and return the old une.
//
// The 'PLATFORM_SPECIFIC_IRQ_OVERHEAD' values correspond to the number of clock cycles
// used by the OS before it reaches the user's service routine.

// As an example, on the Atari, the IRQ service routine looks like this:
//
// PROCESS_IRQ:      cld                [2]
//                   jmp ($216)         [5]
//
// This means that 'set_irq_vector_address' changes the generic IRQ-processing vector at
// 0x216 (and returns its old value) and the overhead for reaching user code is 7 cycles.

uint8_t * set_irq_vector_address(uint8_t * newvec);

#define PLATFORM_SPECIFIC_IRQ_OVERHEAD_ATARI  7
#define PLATFORM_SPECIFIC_IRQ_OVERHEAD_C64    (C64_IRQ_OVERHEAD_TBD)
#define PLATFORM_SPECIFIC_IRQ_OVERHEAD_SIM65  (SIM65_IRQ_OVERHEAD_TBD)

#define PLATFORM_SPECIFIC_IRQ_OVERHEAD PLATFORM_SPECIFIC_IRQ_OVERHEAD_ATARI

#endif
