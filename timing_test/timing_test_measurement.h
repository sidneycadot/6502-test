
///////////////////////////////
// timing_test_measurement.h //
///////////////////////////////

#ifndef TIMING_TEST_MEASUREMENT_H
#define TIMING_TEST_MEASUREMENT_H

#include <stdbool.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                       INTERFACE TO LOW-LEVEL MEASUREMENT ROUTINE                                  //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// We interface via global variables because it's faster more compact than passing arguments via the C stack.

typedef enum {
    Par1_OpcodeOffset,
    Par1_ClockCycleCount,
    Par12_OpcodeOffset_Immediate,
    Par12_OpcodeOffset_ZPage,
    Par12_OpcodeOffset_AbsOffset,
    Par12_OpcodeOffset_Displacement,
    Par123_OpcodeOffset_ZPage_XReg,
    Par123_OpcodeOffset_ZPage_YReg,
    Par123_OpcodeOffset_AbsOffset_XReg,
    Par123_OpcodeOffset_AbsOffset_YReg,
    Par123_OpcodeOffset_ZPage_AbsOffset,
    Par123_OpcodeOffset_BranchDisplacement_TakenNotTaken,
    Par1234_Generic,
    Par1234_OpcodeOffset_ZPage_XReg_AbsOffset,
    Par1234_OpcodeOffset_ZPage_AbsOffset_YReg,
    Par1234_OpcodeOffset_ZPage_BranchDisplacement_TakenNotTaken
} ParSpec;

extern uint8_t par1;
extern uint8_t par2;
extern uint8_t par3;
extern uint8_t par4;

extern uint8_t num_zpage_preserve; // How many zero-pages addresses should the test preserve?
extern uint8_t zpage_preserve[2];  // Zero page addresses to preserve while the test executes (0, 1, or 2 values).

extern unsigned m_test_overhead_cycles;
extern unsigned m_instruction_cycles;

extern unsigned long test_count;
extern unsigned long msm_count;
extern unsigned long error_count;

#define F_NONE             0
#define F_STOP_ON_ERROR    0x01

void reset_test_counts(void);
void prepare_opcode_tests_skip(const char * test_description);
void prepare_opcode_tests(const char * test_description, ParSpec parspec);
bool execute_single_opcode_test(uint8_t * entrypoint, uint8_t flags);
void report_test_counts(void);

#endif
