
////////////////////////////
// timing_test_routines.h //
////////////////////////////

#ifndef TIMING_TEST_ROUTINES_H
#define TIMING_TEST_ROUTINES_H

#include <stdint.h>
#include <stdbool.h>

// For testing purposes we often want to loop over values in the range 0..255. The STEP_SIZE variable
// determines the steps to take; in case it's one, all 256 values are visited.
//
// By picking a smaller value, we can loop over the range faster (by skipping many values). For example,
// if STEP_SIZE is 17, we only loop over {0, 17, 34, ..., 238, 255} -- just 16 out of the 256 values.
//
// The STEP_SIZE determines how rigorous the tests are. Effectively, at bigger sizes of STEP_SIZE,
// a smaller part of the "test space" is covered.
//
// It is recommended to pick this value as a divisor of 255, so one of 1, 3, 5, 15, 17, 51, 85, and 255.

extern unsigned STEP_SIZE;

// Timing test for skipped instructions.

bool timing_test_skip_instruction (const char * opcode_description, uint8_t opcode);

// Timing tests for one, two, and three-byte instruction sequences.

bool timing_test_single_byte_instruction_sequence (const char * opcode_description,                             uint8_t b1                                                       , unsigned instruction_cycles);
bool timing_test_two_byte_instruction_sequence    (const char * opcode_description, uint8_t test_opcode_offset, uint8_t b1, uint8_t b2            , unsigned test_overhead_cycles, unsigned instruction_cycles);
bool timing_test_three_byte_instruction_sequence  (const char * opcode_description, uint8_t test_opcode_offset, uint8_t b1, uint8_t b2, uint8_t b3, unsigned test_overhead_cycles, unsigned instruction_cycles);

// Timing tests for read-instructions.

bool timing_test_read_immediate_instruction       (const char * opcode_description, uint8_t opcode);
bool timing_test_read_zpage_instruction           (const char * opcode_description, uint8_t opcode);
bool timing_test_read_zpage_x_instruction         (const char * opcode_description, uint8_t opcode);
bool timing_test_read_zpage_y_instruction         (const char * opcode_description, uint8_t opcode);
bool timing_test_read_abs_instruction             (const char * opcode_description, uint8_t opcode);
bool timing_test_read_abs_instruction_slow        (const char * opcode_description, uint8_t opcode);
bool timing_test_read_abs_x_instruction           (const char * opcode_description, uint8_t opcode);
bool timing_test_read_abs_y_instruction           (const char * opcode_description, uint8_t opcode);
bool timing_test_read_abs_y_instruction_save_sp   (const char * opcode_description, uint8_t opcode);
bool timing_test_read_zpage_x_indirect_instruction(const char * opcode_description, uint8_t opcode);
bool timing_test_read_zpage_indirect_y_instruction(const char * opcode_description, uint8_t opcode);
bool timing_test_read_zpage_indirect_instruction  (const char * opcode_description, uint8_t opcode);

// Timing tests for write-instructions.

bool timing_test_write_zpage_instruction                   (const char * opcode_description, uint8_t opcode);
bool timing_test_write_zpage_x_instruction                 (const char * opcode_description, uint8_t opcode);
bool timing_test_write_zpage_y_instruction                 (const char * opcode_description, uint8_t opcode);
bool timing_test_write_abs_instruction                     (const char * opcode_description, uint8_t opcode);
bool timing_test_write_abs_x_instruction                   (const char * opcode_description, uint8_t opcode);
bool timing_test_write_abs_x_instruction_shy_absx          (const char * opcode_description, uint8_t opcode);
bool timing_test_write_abs_y_instruction                   (const char * opcode_description, uint8_t opcode);
bool timing_test_write_abs_y_instruction_sha_absy          (const char * opcode_description, uint8_t opcode);
bool timing_test_write_abs_y_instruction_shx_absy          (const char * opcode_description, uint8_t opcode);
bool timing_test_write_abs_y_instruction_tas_absy          (const char * opcode_description, uint8_t opcode);
bool timing_test_write_abs_y_instruction_save_sp           (const char * opcode_description, uint8_t opcode);
bool timing_test_write_zpage_x_indirect_instruction        (const char * opcode_description, uint8_t opcode);
bool timing_test_write_zpage_indirect_y_instruction        (const char * opcode_description, uint8_t opcode);
bool timing_test_write_zpage_indirect_y_instruction_sha_zpy(const char * opcode_description, uint8_t opcode);
bool timing_test_write_zpage_indirect_instruction          (const char * opcode_description, uint8_t opcode);

// Timing tests for read-modify-write instructions.

bool timing_test_read_modify_write_zpage_instruction           (const char * opcode_description, uint8_t opcode);
bool timing_test_read_modify_write_zpage_x_instruction         (const char * opcode_description, uint8_t opcode);
bool timing_test_read_modify_write_abs_instruction             (const char * opcode_description, uint8_t opcode);
bool timing_test_read_modify_write_abs_x_instruction_v1        (const char * opcode_description, uint8_t opcode);
bool timing_test_read_modify_write_abs_x_instruction_v2        (const char * opcode_description, uint8_t opcode);
bool timing_test_read_modify_write_abs_y_instruction           (const char * opcode_description, uint8_t opcode);
bool timing_test_read_modify_write_zpage_x_indirect_instruction(const char * opcode_description, uint8_t opcode);
bool timing_test_read_modify_write_zpage_indirect_y_instruction(const char * opcode_description, uint8_t opcode);

// Timing tests for branch, jump, and interrupt-related instructions.

bool timing_test_branch_instruction       (const char * opcode_description, uint8_t opcode, bool branch_when_flag_set);
bool timing_test_bit_branch_instruction   (const char * opcode_description, uint8_t opcode, bool branch_when_bit_set);
bool timing_test_branch_always_instruction(const char * opcode_description, uint8_t opcode);

bool timing_test_jmp_abs_instruction            (const char * opcode_description);
bool timing_test_jmp_abs_indirect_instruction   (const char * opcode_description);
bool timing_test_jmp_abs_x_indirect_instruction (const char * opcode_description);

bool timing_test_jsr_abs_instruction      (const char * opcode_description);
bool timing_test_rts_instruction          (const char * opcode_description);

bool timing_test_brk_instruction          (const char * opcode_description);
bool timing_test_rti_instruction          (const char * opcode_description);

#endif
