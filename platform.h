
////////////////
// platform.h //
////////////////

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////
//                                                                    //
// PROTOTYPES FOR FUNCTIONS THAT ARE IMPLEMENTED IN platform_<name>.s //
//                                                                    //
////////////////////////////////////////////////////////////////////////

// Enable/disable DMA and interrupts, to create a situation where the 6502 timing behaves in a way that
// allows the 'measure_cycles' and 'measure_cycles_zp_safe' to do their job.

void __fastcall__ dma_and_interrupts_off(void);
void __fastcall__ dma_and_interrupts_on(void);

// Indicate which zero-page addresses can be touched by the testing code.
bool __fastcall__ zp_address_is_safe(uint8_t address);

// Measure the number of cycles that a code fragment will take, up to (but not including) a final RTS.

int16_t __fastcall__ measure_cycles(uint8_t * code);

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

uint8_t * __fastcall__  set_irq_vector_address(uint8_t * newvec);

/////////////////////////////////////////////////////////////////////////
//                                                                     //
// PROTOTYPES FOR FUNCTIONS THAT ARE IMPLEMENTED IN platform_generic.s //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

int16_t __fastcall__ measure_cycles_zp_safe(uint8_t * code);

/////////////////////////////////////////////////////////////////////////
//                                                                     //
//                     PLATFORM-DEPENDENT CONSTANTS                    //
//                                                                     //
/////////////////////////////////////////////////////////////////////////

# if defined(TIC_TARGET_ATARI)
#     define PLATFORM_SPECIFIC_IRQ_OVERHEAD 7
# elif defined(TIC_TARGET_C64)
#     define PLATFORM_SPECIFIC_IRQ_OVERHEAD 28
# elif defined(TIC_TARGET_SIM6502)
#     define PLATFORM_SPECIFIC_IRQ_OVERHEAD 0
# else
#     error "Undefined platform."
# endif
