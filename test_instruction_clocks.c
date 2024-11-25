
///////////////////////////////
// test_instruction_clocks.c //
///////////////////////////////

// This program tests the instruction timing (i.e., clock-cycle counts) of
// the 151 documented 6502 instructions.
//
// This program was validated on physical hardware (for example, an Atari 800 XL and a
// Commodore 64); those show zero errors.
//
// With that fact in mind, the program can serve as a validation reference for emulators
// that try to emulate an entire machine, or only the 6502.
//
// The program depends on the availability of four external functions. Three of those
// are specific to the hardware platform on which the program is run:
//
// * dma_and_interrupts_off()   This create an environment where the measure_cycles()
//                              and measure_cycles_zp_safe() functions can do their work.
//                              On most hardware, this is a matter of disabling video DMA
//                              and interrupts; hence the name.
//
// * dma_and_interrupts_on()    Restore a "normal" environment, where DMA, interrupts, and
//                              any other timing disturbances are once again allowed.
//
// * measure_cycles()           Measure the number of clock cycles needed to execute a short
//                              sequence of 6502 instructions.
//
//                              The length of instruction sequences that can be timed varies
//                              between hardware platforms. The current implementation on the
//                              Atari, for example, can only measure instruction sequences
//                              reliably up to 27 clock cycles.
//
// A fourth external function is needed to accommodate tests that can write to the zero page,
// which may be in use by the 6502 machine's operating system. This function depends on the
// platform dependent measure_cycles() routine, but is itself platform independent. It is a
// drop-in replacement for the standard 'measure_cycles' for cases when this zero-page
// preservation behavior is needed:
//
// * measure_cycles_zp_safe()   Save the contents of the zero page, execute measure_cycles(),
//                              then restore the content of the zero page.
//
// ASSUMPTIONS
// -----------
//
// * The program assumes that the 6502 instructions work correctly, other than that their clock
//   cycle counts may be off.
//
// * Later tests assume that the clock cycle counts of 6502 instructions that were timed
//   by earlier tests for simpler instructions are correct.
//
// * The 65C02 changes (fixes) the behavior of the C and V flags when doing addition or subtraction
//   (ADC or SBC) in decimal mode, at the cost of an extra clock cycle. This will currently not
//   be tested, as the tests are all run with decimal mode disabled. We will add explicit tests
//   for this once we have a 65C02 system available.

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "timing_test_memory.h"
#include "timing_test_routines.h"
#include "timing_test_report.h"
#include "measure_cycles.h"

void generate_code(uint8_t * code, unsigned cycles)
{
    assert(cycles != 1);

    while (cycles != 0)
    {
        if (cycles % 2 != 0)
        {
            *code++ = 0xa5; // LDA 0 (zp)       [3]
            *code++ = 0x00;
            cycles -= 3;
        }
        else
        {
            *code++ = 0xea; // NOP              [2]
            cycles -= 2;
        }
    }
    *code++ = 0x60; // RTS
}

void run_measurement_test(unsigned repeats, unsigned min_cycle_count, unsigned max_cycle_count)
{
    unsigned instruction_cycles, actual_cycles, rep;

    for (rep = 1; rep <= repeats; ++rep)
    {
        for (instruction_cycles = min_cycle_count; instruction_cycles <= max_cycle_count; ++instruction_cycles)
        {
            if (instruction_cycles == 1)
            {
                // Cannot generate 1-cycle test code.
                continue;
            }

            generate_code(TESTCODE_BASE, instruction_cycles);

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(TESTCODE_BASE);
            dma_and_interrupts_on();

            test_report(
                "measurement test",
                0,
                instruction_cycles,
                actual_cycles,
                NULL
            );
        }
    }
}

void timing_test_branch_instructions(void)
{
    // Test branch instructions on the sign a.k.a. negative (N) flag.

    timing_test_branch_taken_instruction    ("BPL, taken"    , 0x10, false);
    timing_test_branch_not_taken_instruction("BPL, not taken", 0x10, true );
    timing_test_branch_taken_instruction    ("BMI, taken"    , 0x30, true );
    timing_test_branch_not_taken_instruction("BMI, not taken", 0x30, false);

    // Test branch instructions on the overflow (V) flag.

    timing_test_branch_taken_instruction    ("BVC, taken"    , 0x50, false);
    timing_test_branch_not_taken_instruction("BVC, not taken", 0x50, true );
    timing_test_branch_taken_instruction    ("BVS, taken"    , 0x70, true );
    timing_test_branch_not_taken_instruction("BVS, not taken", 0x70, false);

    // Test branch instructions on the carry (C) flag.

    timing_test_branch_taken_instruction    ("BCC, taken"    , 0x90, false);
    timing_test_branch_not_taken_instruction("BCC, not taken", 0x90, true );
    timing_test_branch_taken_instruction    ("BCS, taken"    , 0xb0, true );
    timing_test_branch_not_taken_instruction("BCS, not taken", 0xb0, false);

    // Test branch instructions on the zero (Z) flag.

    timing_test_branch_taken_instruction    ("BNE, taken"    , 0xd0, false);
    timing_test_branch_not_taken_instruction("BNE, not taken", 0xd0, true);
    timing_test_branch_taken_instruction    ("BEQ, taken"    , 0xf0, true);
    timing_test_branch_not_taken_instruction("BEQ, not taken", 0xf0, false);
}

void timing_test_single_byte_two_cycle_implied_instructions(void)
{
    // Test timing of the 18 single-byte, 2-cycle "implied" instructions.

    // CLV, CLC, SEC

    timing_test_single_byte_instruction_sequence("CLV", 0xb8, 2);
    timing_test_single_byte_instruction_sequence("CLC", 0x18, 2);
    timing_test_single_byte_instruction_sequence("SEC", 0x38, 2);

    // CLI, SEI
    //
    // Note: These change CPU the interrupt flag, which may be critical to guarantee the proper working
    // of the external environment. For that reason, we sandwich these between PHP/PLP instructions.

    timing_test_three_byte_instruction_sequence("CLI, between PHP and PLP", 0x08, 0x58, 0x28, 3 + 4, 2);
    timing_test_three_byte_instruction_sequence("SEI, between PHP and PLP", 0x08, 0x78, 0x28, 3 + 4, 2);

    // CLD, SED
    // Note: The "SED" is tested in the SED,CLD combination, to prevent that the test leaves decimal mode enabled.

    timing_test_single_byte_instruction_sequence("CLD", 0xd8, 2);
    timing_test_two_byte_instruction_sequence   ("SED, followed by CLD", 0xf8, 0xd8, 2, 2);

    // INX, DEX
    // INY, DEY

    timing_test_single_byte_instruction_sequence("DEY", 0x88, 2);
    timing_test_single_byte_instruction_sequence("INY", 0xc8, 2);
    timing_test_single_byte_instruction_sequence("DEX", 0xca, 2);
    timing_test_single_byte_instruction_sequence("INX", 0xe8, 2);

    // TAY, TYA
    // TAX, TXA

    timing_test_single_byte_instruction_sequence("TYA", 0x98, 2);
    timing_test_single_byte_instruction_sequence("TAY", 0xa8, 2);
    timing_test_single_byte_instruction_sequence("TXA", 0x8a, 2);
    timing_test_single_byte_instruction_sequence("TAX", 0xaa, 2);

    // TSX, TXS

    timing_test_single_byte_instruction_sequence("TSX", 0xba, 2);
    timing_test_two_byte_instruction_sequence   ("TXS, preceded by TSX", 0xba, 0x9a, 2, 2);

    // ASL, ROL, LSR, ROR on the accumulator register.

    timing_test_single_byte_instruction_sequence("ASL", 0x0a, 2);
    timing_test_single_byte_instruction_sequence("ROL", 0x2a, 2);
    timing_test_single_byte_instruction_sequence("LSR", 0x4a, 2);
    timing_test_single_byte_instruction_sequence("ROR", 0x6a, 2);

    // NOP

    timing_test_single_byte_instruction_sequence("NOP", 0xea, 2);
}

void timing_test_single_byte_stack_instructions(void)
{
    // Test the single-byte, 6-cycle push/pull stack instructions.
    //
    // TSX, PHA, TXS              The stack pointer is saved before, and restored after the instruction.

    timing_test_three_byte_instruction_sequence("PHA, between TSX and TXS", 0xba, 0x48, 0x9a, 2 + 2, 3);

    // TSX, PHP, TXS              The stack pointer is saved before, and restored after the instruction.

    timing_test_three_byte_instruction_sequence("PHP, between TSX and TXS", 0xba, 0x08, 0x9a, 2 + 2, 3);

    // PHA, PLA                   The value to be pulled is pushed immediately before.

    timing_test_two_byte_instruction_sequence("PLA, preceded by PHA", 0x48, 0x68, 3, 4);

    // PHP, PLP                   The value to be pulled is pushed immediately before.

    timing_test_two_byte_instruction_sequence("PLP, preceded by PHP", 0x08, 0x28, 3, 4);
}

void timing_test_read_immediate_instructions(void)
{
    // The 11 immediate-mode instructions all take 2 cycles.

    timing_test_read_immediate_instruction("LDY #imm", 0xa0);
    timing_test_read_immediate_instruction("LDX #imm", 0xa2);
    timing_test_read_immediate_instruction("CPY #imm", 0xc0);
    timing_test_read_immediate_instruction("CPX #imm", 0xe0);

    timing_test_read_immediate_instruction("ORA #imm", 0x09);
    timing_test_read_immediate_instruction("AND #imm", 0x29);
    timing_test_read_immediate_instruction("EOR #imm", 0x49);
    timing_test_read_immediate_instruction("ADC #imm", 0x69);
    timing_test_read_immediate_instruction("LDA #imm", 0xa9);
    timing_test_read_immediate_instruction("CMP #imm", 0xc9);
    timing_test_read_immediate_instruction("SBC #imm", 0xe9);
}

void timing_test_read_zpage_instructions(void)
{
    // The 12 read-from-zero-page instructions all take 3 cycles.

    timing_test_read_zpage_instruction("BIT zpage", 0x24);
    timing_test_read_zpage_instruction("LDX zpage", 0xa6);
    timing_test_read_zpage_instruction("LDY zpage", 0xa4);
    timing_test_read_zpage_instruction("CPX zpage", 0xe4);
    timing_test_read_zpage_instruction("CPY zpage", 0xc4);

    timing_test_read_zpage_instruction("ORA zpage", 0x05);
    timing_test_read_zpage_instruction("AND zpage", 0x25);
    timing_test_read_zpage_instruction("EOR zpage", 0x45);
    timing_test_read_zpage_instruction("ADC zpage", 0x65);
    timing_test_read_zpage_instruction("LDA zpage", 0xa5);
    timing_test_read_zpage_instruction("CMP zpage", 0xc5);
    timing_test_read_zpage_instruction("SBC zpage", 0xe5);
}

void timing_test_read_zpage_x_instructions(void)
{
    // The 8 read-from-zero-page-with-x-indexing instructions all take 4 cycles.

    timing_test_read_zpage_x_instruction("LDY zpage,X", 0xb4);

    timing_test_read_zpage_x_instruction("ORA zpage,X", 0x15);
    timing_test_read_zpage_x_instruction("AND zpage,X", 0x35);
    timing_test_read_zpage_x_instruction("EOR zpage,X", 0x55);
    timing_test_read_zpage_x_instruction("ADC zpage,X", 0x75);
    timing_test_read_zpage_x_instruction("LDA zpage,X", 0xb5);
    timing_test_read_zpage_x_instruction("CMP zpage,X", 0xd5);
    timing_test_read_zpage_x_instruction("SBC zpage,X", 0xf5);
}

void timing_test_read_zpage_y_instructions(void)
{
    // The 1 read-from-zero-page-with-y-indexing instruction takes 4 cycles.

    timing_test_read_zpage_y_instruction("LDX zpage,Y", 0xb6);
}

void timing_test_read_abs_instructions(void)
{
    // The 12 read-from-absolute-address instructions all take 4 cycles.

    timing_test_read_abs_instruction("BIT abs", 0x2c);
    timing_test_read_abs_instruction("LDX abs", 0xae);
    timing_test_read_abs_instruction("LDY abs", 0xac);
    timing_test_read_abs_instruction("CPX abs", 0xec);
    timing_test_read_abs_instruction("CPY abs", 0xcc);

    timing_test_read_abs_instruction("ORA abs", 0x0d);
    timing_test_read_abs_instruction("AND abs", 0x2d);
    timing_test_read_abs_instruction("EOR abs", 0x4d);
    timing_test_read_abs_instruction("ADC abs", 0x6d);
    timing_test_read_abs_instruction("LDA abs", 0xad);
    timing_test_read_abs_instruction("CMP abs", 0xcd);
    timing_test_read_abs_instruction("SBC abs", 0xed);
}

void timing_test_read_abs_x_instructions(void)
{
    // The 8 read-from-absolute-addressing-with-x-indexing instructions take 4 or 5 cycles.
    // An extra cycle is added in case the indexing with X causes the effective address
    // to be on a different page than the base address.

    timing_test_read_abs_x_instruction("LDY abs,X", 0xbc);

    timing_test_read_abs_x_instruction("ORA abs,X", 0x1d);
    timing_test_read_abs_x_instruction("AND abs,X", 0x3d);
    timing_test_read_abs_x_instruction("EOR abs,X", 0x5d);
    timing_test_read_abs_x_instruction("ADC abs,X", 0x7d);
    timing_test_read_abs_x_instruction("LDA abs,X", 0xbd);
    timing_test_read_abs_x_instruction("CMP abs,X", 0xdd);
    timing_test_read_abs_x_instruction("SBC abs,X", 0xfd);
}

void timing_test_read_abs_y_instructions(void)
{
    // The 8 read-from-absolute-addressing-with-y-indexing instructions take 4 or 5 cycles.
    // An extra cycle is added in case the indexing with Y causes the effective address
    // to be on a different page than the base address.

    timing_test_read_abs_y_instruction("LDX abs,Y", 0xbe);

    timing_test_read_abs_y_instruction("ORA abs,Y", 0x19);
    timing_test_read_abs_y_instruction("AND abs,Y", 0x39);
    timing_test_read_abs_y_instruction("EOR abs,Y", 0x59);
    timing_test_read_abs_y_instruction("ADC abs,Y", 0x79);
    timing_test_read_abs_y_instruction("LDA abs,Y", 0xb9);
    timing_test_read_abs_y_instruction("CMP abs,Y", 0xd9);
    timing_test_read_abs_y_instruction("SBC abs,Y", 0xf9);
}

void timing_test_read_zpage_x_indirect_instructions(void)
{
    // The 7 write-to-zero-page instructions all take 6 cycles.

    timing_test_read_zpage_x_indirect_instruction("ORA (zpage,X)", 0x01);
    timing_test_read_zpage_x_indirect_instruction("AND (zpage,X)", 0x21);
    timing_test_read_zpage_x_indirect_instruction("EOR (zpage,X)", 0x41);
    timing_test_read_zpage_x_indirect_instruction("ADC (zpage,X)", 0x61);
    timing_test_read_zpage_x_indirect_instruction("LDA (zpage,X)", 0xa1);
    timing_test_read_zpage_x_indirect_instruction("CMP (zpage,X)", 0xc1);
    timing_test_read_zpage_x_indirect_instruction("SBC (zpage,X)", 0xe1);
}

void timing_test_read_zpage_indirect_y_instructions(void)
{
    // The 7 write-to-zero-page instructions all take 6 cycles.

    timing_test_read_zpage_indirect_y_instruction("ORA (zpage),Y", 0x11);
    timing_test_read_zpage_indirect_y_instruction("AND (zpage),Y", 0x31);
    timing_test_read_zpage_indirect_y_instruction("EOR (zpage),Y", 0x51);
    timing_test_read_zpage_indirect_y_instruction("ADC (zpage),Y", 0x71);
    timing_test_read_zpage_indirect_y_instruction("LDA (zpage),Y", 0xb1);
    timing_test_read_zpage_indirect_y_instruction("CMP (zpage),Y", 0xd1);
    timing_test_read_zpage_indirect_y_instruction("SBC (zpage),Y", 0xf1);
}

void timing_test_write_zpage_instructions(void)
{
    // The 3 write-to-zero-page instructions all take 3 cycles.

    timing_test_write_zpage_instruction("STA zpage", 0x85);
    timing_test_write_zpage_instruction("STX zpage", 0x86);
    timing_test_write_zpage_instruction("STY zpage", 0x84);
}

void timing_test_write_zpage_x_instructions(void)
{
    // The 3 write-to-zero-page-with-x-indexing instructions all take 4 cycles.

    timing_test_write_zpage_x_instruction("STA zpage,X", 0x95);
    timing_test_write_zpage_x_instruction("STY zpage,X", 0x94);
}

void timing_test_write_zpage_y_instructions(void)
{
    // The 3 write-to-zero-page-with-y-indexing instructions all take 4 cycles.

    timing_test_write_zpage_y_instruction("STX zpage,Y", 0x96);
}

void timing_test_write_abs_instructions(void)
{
    // The 3 write-to-absolute-address instructions all take 4 cycles.

    timing_test_write_abs_instruction("STA abs", 0x8d);
    timing_test_write_abs_instruction("STX abs", 0x8e);
    timing_test_write_abs_instruction("STY abs", 0x8c);
}

void timing_test_write_abs_x_instructions(void)
{
    // The 1 write-to-absolute-address-with-x-indexing instructions all take 5 cycles.

    timing_test_write_abs_x_instruction("STA abs,X", 0x9d);
}

void timing_test_write_abs_y_instructions(void)
{
    // The 1 write-to-absolute-address-with-y-indexing instructions all take 5 cycles.

    timing_test_write_abs_y_instruction("STA abs,Y", 0x99);
}

void timing_test_write_zpage_x_indirect_instructions(void)
{
    // The 1 write-to-zero-page instructions takes 6 cycles.

    timing_test_write_zpage_x_indirect_instruction("STA (zpage,X)", 0x81);
}

void timing_test_write_zpage_indirect_y_instructions(void)
{
    // The 1 write-to-zero-page instructions takes 6 cycles.

    timing_test_write_zpage_indirect_y_instruction("STA (zpage),Y", 0x91);
}

void timing_test_read_modify_write_zpage_instructions(void)
{
    // The 6 read-modify-write-to-zero-page instructions all take 5 cycles.

    timing_test_read_modify_write_zpage_instruction("ASL zpage", 0x06);
    timing_test_read_modify_write_zpage_instruction("ROL zpage", 0x26);
    timing_test_read_modify_write_zpage_instruction("LSR zpage", 0x46);
    timing_test_read_modify_write_zpage_instruction("ROR zpage", 0x66);
    timing_test_read_modify_write_zpage_instruction("DEC zpage", 0xc6);
    timing_test_read_modify_write_zpage_instruction("INC zpage", 0xe6);
}

void timing_test_read_modify_write_zpage_x_instructions(void)
{
    // The 6 read-modify-write-to-zero-page instructions all take 6 cycles.

    timing_test_read_modify_write_zpage_x_instruction("ASL zpage,X", 0x16);
    timing_test_read_modify_write_zpage_x_instruction("ROL zpage,X", 0x36);
    timing_test_read_modify_write_zpage_x_instruction("LSR zpage,X", 0x56);
    timing_test_read_modify_write_zpage_x_instruction("ROR zpage,X", 0x76);
    timing_test_read_modify_write_zpage_x_instruction("DEC zpage,X", 0xd6);
    timing_test_read_modify_write_zpage_x_instruction("INC zpage,X", 0xf6);
}

void timing_test_read_modify_write_abs_instructions(void)
{
    // The 6 read-modify-write-to-absolute-address instructions all take 6 cycles.

    timing_test_read_modify_write_abs_instruction("ASL abs", 0x0e);
    timing_test_read_modify_write_abs_instruction("ROL abs", 0x2e);
    timing_test_read_modify_write_abs_instruction("LSR abs", 0x4e);
    timing_test_read_modify_write_abs_instruction("ROR abs", 0x6e);
    timing_test_read_modify_write_abs_instruction("DEC abs", 0xce);
    timing_test_read_modify_write_abs_instruction("INC abs", 0xee);
}

void timing_test_read_modify_write_abs_x_instructions(void)
{
    // The 6 read-modify-write-to-absolute-address instructions all take 7 cycles.

    timing_test_read_modify_write_abs_x_instruction("ASL abs,X", 0x1e);
    timing_test_read_modify_write_abs_x_instruction("ROL abs,X", 0x3e);
    timing_test_read_modify_write_abs_x_instruction("LSR abs,X", 0x5e);
    timing_test_read_modify_write_abs_x_instruction("ROR abs,X", 0x7e);
    timing_test_read_modify_write_abs_x_instruction("DEC abs,X", 0xde);
    timing_test_read_modify_write_abs_x_instruction("INC abs,X", 0xfe);
}

void run_cpu_test(void)
{
    // Test the timing of the 8 branch instructions.
    timing_test_branch_instructions();

    // Test the timing of the 22 single-byte, two-cycle opcodes.
    timing_test_single_byte_two_cycle_implied_instructions();

    // Test the timing of the 4 stack push/pull instructions.
    timing_test_single_byte_stack_instructions();

    // Test the timing of the 11 two-byte immediate-mode instructions.
    timing_test_read_immediate_instructions();

    // Test the timing of the 12 two-byte read-from-zero-page instructions.
    timing_test_read_zpage_instructions();

    // Test the timing of the 8 two-byte read-from-zero-page-with-x-indexing instructions.
    timing_test_read_zpage_x_instructions();

    // Test the timing of the 1 two-byte read-from-zero-page-with-y-indexing instruction.
    timing_test_read_zpage_y_instructions();

    // Test the timing of the 12 two-byte read-from-abs-address instructions.
    timing_test_read_abs_instructions();

    // Test the timing of the 8 three-byte read-from-zero-page-with-x-indexing instructions.
    timing_test_read_abs_x_instructions();

    // Test the timing of the 8 three-byte read-from-zero-page-with-y-indexing instructions.
    timing_test_read_abs_y_instructions();

    // Test the timing of the 7 two-byte read-zpage_with-x-indexing-indirect instructions.
    timing_test_read_zpage_x_indirect_instructions();

    // Test the timing of the 7 two-byte read-zpage_indirect_with-y-indexing instructions.
    timing_test_read_zpage_indirect_y_instructions();

    // Test the timing of the 3 two-byte write-to-zero-page instructions.
    timing_test_write_zpage_instructions();

    // Test the timing of the 2 two-byte write-to-zero-page-with-x-indexing instructions.
    timing_test_write_zpage_x_instructions();

    // Test the timing of the 1 two-byte write-to-zero-page-with-y-indexing instruction.
    timing_test_write_zpage_y_instructions();

    // Test the timing of the 3 three-byte write-to-absolute-address instructions.
    timing_test_write_abs_instructions();

    // Test the timing of the 1 three-byte write-to-absolute-address-with-x-indexing instruction.
    timing_test_write_abs_x_instructions();

    // Test the timing of the 1 three-byte write-to-absolute-address-with-y-indexing instruction.
    timing_test_write_abs_y_instructions();

    // Test the timing of the 7 two-byte read-zpage_with-x-indexing-indirect instructions.
    timing_test_write_zpage_x_indirect_instructions();

    // Test the timing of the 7 two-byte read-zpage_indirect_with-y-indexing instructions.
    timing_test_write_zpage_indirect_y_instructions();

    // Test the timing of the 6 two-byte read-modify-write-zpage instructions.
    timing_test_read_modify_write_zpage_instructions();

    // Test the timing of the 6 two-byte read-modify-write-zpage-with-x-indexing instructions.
    timing_test_read_modify_write_zpage_x_instructions();

    // Test the timing of the 6 three-byte read-modify-write-absolute-address instructions.
    timing_test_read_modify_write_abs_instructions();

    // Test the timing of the 6 three-byte read-modify-write-absolute-address-with-x-indexing instructions.
    timing_test_read_modify_write_abs_x_instructions();

    // Test the timing of the 6 remaining instructions.

    timing_test_jmp_abs_instruction("JMP abs");
    timing_test_jmp_indirect_instruction("JMP (ind)");

    //timing_test_jsr_abs_instruction("JSR abs");
    //timing_test_rts_instruction("RTS");

    //timing_test_brk_instruction("BRK");
    //timing_test_rti_instruction("RTI");
}

int command_line_loop(void)
{
    char command[80];
    unsigned par1, par2, par3;
    int result;

    result = allocate_testcode_block(2048);
    if (result != 0)
    {
        puts("Unable to allocate TESTCODE block.");
        return -1;
    }

    printf("Test memory was allocated as follows:\n");
    printf("\n");
    printf("  TESTCODE_PTR     %p\n", TESTCODE_PTR);
    printf("  TESTCODE_BASE    %p\n", TESTCODE_BASE);
    printf("  TESTCODE_ANCHOR  %p\n", TESTCODE_ANCHOR);
    printf("  TESTCODE_LAST    %p\n", TESTCODE_LAST);
    printf("\n");

    for (;;)
    {
        printf("Enter command (or ENTER for help)\n");
        fgets(command, sizeof(command), stdin);

        if (strcmp(command, "quit") == 0)
        {
            break;
        }
        else if (sscanf(command, "msm %u %u %u", &par1, &par2, &par3) == 3)
        {
            printf("\n");
            printf("Performing measurement tests ...\n");

            test_count = 0;
            error_count = 0;

            run_measurement_test(par1, par2, par3);

            printf("Tests performed ...... : %lu\n", test_count);
            printf("Tests failed ......... : %lu\n", error_count);
            printf("\n");
        }
        else if (sscanf(command, "cpu %u", &par1) == 1)
        {
            const unsigned lookup_table[8] = {1, 3, 5, 15, 17, 51, 85, 255};

            printf("\n");

            if (par1 > 7)
            {
                printf("Level must be 0 to 7. Higher values\n");
                printf("test more cases, but are MUCH slower.\n");
                printf("\n");
                continue;
            }

            STEP_SIZE = lookup_table[7 - par1];

            printf("Starting level %u test (step size %u).\n", par1, STEP_SIZE);
            printf("\n");

            test_count = 0;
            error_count = 0;

            run_cpu_test();

            printf("\n");
            printf("Tests performed ...... : %lu\n", test_count);
            printf("Tests failed ......... : %lu\n", error_count);
            printf("\n");
        }
        else
        {
            printf("Commands:\n");
            printf("\n");
            printf("> msm <nreps> <min_c> <max_c>\n");
            printf("\n");
            printf("  Test measurement functionality.\n");
            printf("\n");
            printf("  * nreps: number of repeats\n");
            printf("  * min_c: min number of cycles to test\n");
            printf("  * max_c: max number of cycles to test\n");
            printf("\n");
            printf("> cpu <level>\n");
            printf("\n");
            printf("  Test timing of 6502 instructions.\n");
            printf("\n");
            printf("  * level: 0 (fast) to 7 (slow)\n");
            printf("\n");
            printf("> quit\n");
            printf("\n");
            printf("  Quit the program.\n");
            printf("\n");
        }
    }

    free_testcode_block();

    return 0; // Report success.
}

int main(void)
{
    int result;

    printf("*** TIC v0.2.1 ***\n");
    printf("\n");
    result = command_line_loop();

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
