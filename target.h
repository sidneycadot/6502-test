
//////////////
// target.h //
//////////////

#ifndef TARGET_H
#define TARGET_H

#include <stdint.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//  PROTOTYPES FOR FUNCTIONS THAT ARE IMPLEMENTED (IN EITHER C OR ASSEMBLY) FOR EACH SUPPORTED TARGET  //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// The 'program_start_hook' routine will be called once at startup of the program.
// Use it to clear the screen, set nice colors, and so on.
void __fastcall__ program_start_hook(void);

// This will be called once at the end of the program.
void __fastcall__ program_end_hook(void);

// The 'pre_measurement_hook' routine will be called before zero or more (but typically: dozens
// or hundreds) timing measurements are made. Use it to create an environment where the
// measure_cycles() routine can reliably do its work.
// Typically, this would mean things like disabling interrupts, and disabling video chip DMA.
void __fastcall__ pre_big_measurement_block_hook(void);

// This will be a number of measurements have been completed, and no more measurements are forthcoming soom.
// Use it to undo the actions done in the 'post_measurements_hook' routine.
void __fastcall__ post_big_measurement_block_hook(void);

// This is called before testing a specific opcode.
void __fastcall__ pre_every_test_hook(const char * test_description);

// The 'post_measure_cycles_hook' routine is called immediately following each call to 'measure_cycles'
// or 'measure_cycles_zp_safe'. It reports success, and the test_count and error_count values updated
// for the timing measurement that was executed just before.
//
// If this function returns false, execution will be terminated gracefully.
// This feature can be used to stop a test run in progress.
bool __fastcall__ post_every_measurement_hook(const char * test_description, bool success, unsigned long test_count, unsigned long error_count);

// Enable/disable DMA and interrupts, to create a situation where the 6502 timing behaves in a way that
// allows the 'measure_cycles' and 'measure_cycles_zp_safe' to do their job.

// Indicate which zero-page addresses can be touched by the testing code.
bool __fastcall__ zp_address_is_safe(uint8_t address);

// Report back measurement status.

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

////////////////////////////////////////////////////////////
//                                                        //
//  PROTOTYPES FOR GENERIC TARGET-INDEPENDENT) FUNCTIONS  //
//                                                        //
////////////////////////////////////////////////////////////

int16_t __fastcall__ measure_cycles_zp_safe(uint8_t * code);

///////////////////////////////////////////////////////////
//                                                       //
//  CONSTANTS THAT ARE SPECIFIC TO EACH SUPPRTED TARGET  //
//                                                       //
///////////////////////////////////////////////////////////

# if defined(TIC_TARGET_ATARI)
#     define TARGET_SPECIFIC_IRQ_OVERHEAD 7
# elif defined(TIC_TARGET_C64)
#     define TARGET_SPECIFIC_IRQ_OVERHEAD 28
# else
#     error "No valid target specified."
# endif

#endif
