
////////////////////////
// tic_cmd_cpu_test.c //
////////////////////////

#include <stdio.h>

#include "timing_test_routines.h"
#include "timing_test_measurement.h"
#include "timing_test_zeropage_backup.h"
#include "target.h"

#include "tic_cmd_cpu_test.h"

bool timing_test_stackpointer_transfer_instructions(void)
{
    // Test the 2 instructions to transfer to stack pointer to and from the X register.
    return
        timing_test_single_byte_instruction_sequence("TSX", 0xba, 2) &&
        timing_test_two_byte_instruction_sequence   ("TXS", 0xba, 0x9a, 2, 2);
}

bool timing_test_single_byte_stack_push_pull_instructions(void)
{
    // Test the 4 instructions that push to /pull from the stack.

    return
        // TSX, PHA, TXS - The stack pointer is saved before, and restored after the instruction.
        timing_test_three_byte_instruction_sequence("PHA", 0xba, 0x48, 0x9a, 2 + 2, 3) &&
        // TSX, PHP, TXS - The stack pointer is saved before, and restored after the instruction.
        timing_test_three_byte_instruction_sequence("PHP", 0xba, 0x08, 0x9a, 2 + 2, 3) &&
        // PHA, PLA - The value to be pulled is pushed immediately before.
        timing_test_two_byte_instruction_sequence("PLA", 0x48, 0x68, 3, 4) &&
        // PHP, PLP - The value to be pulled is pushed immediately before.
        timing_test_two_byte_instruction_sequence("PLP", 0x08, 0x28, 3, 4);
}

bool timing_test_single_byte_two_cycle_implied_instructions(void)
{
    // Test 20 single-byte, 2-cycle "implied" instructions.
    // Note: the TSX and TXS instructions are tested separately.

    return
        // CLV, CLC, SEC
        timing_test_single_byte_instruction_sequence("CLV", 0xb8, 2) &&
        timing_test_single_byte_instruction_sequence("CLC", 0x18, 2) &&
        timing_test_single_byte_instruction_sequence("SEC", 0x38, 2) &&
        // CLI, SEI
        //
        // Note: These change the CPU interrupt-disable flag, which may be critical.
        // For that reason, we sandwich them between PHP/PLP instructions.
        timing_test_three_byte_instruction_sequence("CLI", 0x08, 0x58, 0x28, 3 + 4, 2) &&
        timing_test_three_byte_instruction_sequence("SEI", 0x08, 0x78, 0x28, 3 + 4, 2) &&
        // CLD, SED
        // Note: The "SED" is tested in the SED, CLD combination, to prevent that the test leaves decimal mode enabled.
        timing_test_single_byte_instruction_sequence("CLD", 0xd8,          2) &&
        timing_test_two_byte_instruction_sequence   ("SED", 0xf8, 0xd8, 2, 2) &&
        // INX, DEX, INY, DEY
        timing_test_single_byte_instruction_sequence("DEY", 0x88, 2) &&
        timing_test_single_byte_instruction_sequence("INY", 0xc8, 2) &&
        timing_test_single_byte_instruction_sequence("DEX", 0xca, 2) &&
        timing_test_single_byte_instruction_sequence("INX", 0xe8, 2) &&
        // TAY, TYA, TAX, TXA
        timing_test_single_byte_instruction_sequence("TYA", 0x98, 2) &&
        timing_test_single_byte_instruction_sequence("TAY", 0xa8, 2) &&
        timing_test_single_byte_instruction_sequence("TXA", 0x8a, 2) &&
        timing_test_single_byte_instruction_sequence("TAX", 0xaa, 2) &&
        // ASL, ROL, LSR, ROR on the accumulator register.
        timing_test_single_byte_instruction_sequence("ASL A", 0x0a, 2) &&
        timing_test_single_byte_instruction_sequence("ROL A", 0x2a, 2) &&
        timing_test_single_byte_instruction_sequence("LSR A", 0x4a, 2) &&
        timing_test_single_byte_instruction_sequence("ROR A", 0x6a, 2) &&
        // NOP
        timing_test_single_byte_instruction_sequence("NOP", 0xea, 2);
}

bool timing_test_read_immediate_instructions(void)
{
    // The 11 read-immediate instructions take 2 cycles.

    return
        timing_test_read_immediate_instruction("LDY #imm", 0xa0) &&
        timing_test_read_immediate_instruction("LDX #imm", 0xa2) &&
        timing_test_read_immediate_instruction("CPY #imm", 0xc0) &&
        timing_test_read_immediate_instruction("CPX #imm", 0xe0) &&
        //
        timing_test_read_immediate_instruction("ORA #imm", 0x09) &&
        timing_test_read_immediate_instruction("AND #imm", 0x29) &&
        timing_test_read_immediate_instruction("EOR #imm", 0x49) &&
        timing_test_read_immediate_instruction("ADC #imm", 0x69) &&
        timing_test_read_immediate_instruction("LDA #imm", 0xa9) &&
        timing_test_read_immediate_instruction("CMP #imm", 0xc9) &&
        timing_test_read_immediate_instruction("SBC #imm", 0xe9);
}

bool timing_test_read_zpage_instructions(void)
{
    // The 12 read-from-zero-page instructions take 3 cycles.

    return
        timing_test_read_zpage_instruction("BIT zpage", 0x24) &&
        timing_test_read_zpage_instruction("LDX zpage", 0xa6) &&
        timing_test_read_zpage_instruction("LDY zpage", 0xa4) &&
        timing_test_read_zpage_instruction("CPX zpage", 0xe4) &&
        timing_test_read_zpage_instruction("CPY zpage", 0xc4) &&
        //
        timing_test_read_zpage_instruction("ORA zpage", 0x05) &&
        timing_test_read_zpage_instruction("AND zpage", 0x25) &&
        timing_test_read_zpage_instruction("EOR zpage", 0x45) &&
        timing_test_read_zpage_instruction("ADC zpage", 0x65) &&
        timing_test_read_zpage_instruction("LDA zpage", 0xa5) &&
        timing_test_read_zpage_instruction("CMP zpage", 0xc5) &&
        timing_test_read_zpage_instruction("SBC zpage", 0xe5);
}

bool timing_test_read_zpage_x_instructions(void)
{
    // The 8 read-from-zero-page-with-x-indexing instructions take 4 cycles.

    return
        timing_test_read_zpage_x_instruction("LDY zpage,X", 0xb4) &&
        //
        timing_test_read_zpage_x_instruction("ORA zpage,X", 0x15) &&
        timing_test_read_zpage_x_instruction("AND zpage,X", 0x35) &&
        timing_test_read_zpage_x_instruction("EOR zpage,X", 0x55) &&
        timing_test_read_zpage_x_instruction("ADC zpage,X", 0x75) &&
        timing_test_read_zpage_x_instruction("LDA zpage,X", 0xb5) &&
        timing_test_read_zpage_x_instruction("CMP zpage,X", 0xd5) &&
        timing_test_read_zpage_x_instruction("SBC zpage,X", 0xf5);
}

bool timing_test_read_zpage_y_instructions(void)
{
    // The single read-from-zero-page-with-y-indexing instruction takes 4 cycles.

    return
        timing_test_read_zpage_y_instruction("LDX zpage,Y", 0xb6);
}

bool timing_test_read_abs_instructions(void)
{
    // The 12 read-from-absolute-address instructions take 4 cycles.

    return
        timing_test_read_abs_instruction("BIT abs", 0x2c) &&
        timing_test_read_abs_instruction("LDX abs", 0xae) &&
        timing_test_read_abs_instruction("LDY abs", 0xac) &&
        timing_test_read_abs_instruction("CPX abs", 0xec) &&
        timing_test_read_abs_instruction("CPY abs", 0xcc) &&
        //
        timing_test_read_abs_instruction("ORA abs", 0x0d) &&
        timing_test_read_abs_instruction("AND abs", 0x2d) &&
        timing_test_read_abs_instruction("EOR abs", 0x4d) &&
        timing_test_read_abs_instruction("ADC abs", 0x6d) &&
        timing_test_read_abs_instruction("LDA abs", 0xad) &&
        timing_test_read_abs_instruction("CMP abs", 0xcd) &&
        timing_test_read_abs_instruction("SBC abs", 0xed);
}

bool timing_test_read_abs_x_instructions(void)
{
    // The 8 read-from-absolute-addressing-with-x-indexing instructions take 4 or 5 cycles.
    // An extra cycle is added in case the indexing with X causes the effective address
    // to be on a different page than the base address.

    return
        timing_test_read_abs_x_instruction("LDY abs,X", 0xbc) &&
        //
        timing_test_read_abs_x_instruction("ORA abs,X", 0x1d) &&
        timing_test_read_abs_x_instruction("AND abs,X", 0x3d) &&
        timing_test_read_abs_x_instruction("EOR abs,X", 0x5d) &&
        timing_test_read_abs_x_instruction("ADC abs,X", 0x7d) &&
        timing_test_read_abs_x_instruction("LDA abs,X", 0xbd) &&
        timing_test_read_abs_x_instruction("CMP abs,X", 0xdd) &&
        timing_test_read_abs_x_instruction("SBC abs,X", 0xfd);
}

bool timing_test_read_abs_y_instructions(void)
{
    // The 8 read-from-absolute-addressing-with-y-indexing instructions take 4 or 5 cycles.
    // An extra cycle is added in case the indexing with Y causes the effective address
    // to be on a different page than the base address.

    return
        timing_test_read_abs_y_instruction("LDX abs,Y", 0xbe) &&
        //
        timing_test_read_abs_y_instruction("ORA abs,Y", 0x19) &&
        timing_test_read_abs_y_instruction("AND abs,Y", 0x39) &&
        timing_test_read_abs_y_instruction("EOR abs,Y", 0x59) &&
        timing_test_read_abs_y_instruction("ADC abs,Y", 0x79) &&
        timing_test_read_abs_y_instruction("LDA abs,Y", 0xb9) &&
        timing_test_read_abs_y_instruction("CMP abs,Y", 0xd9) &&
        timing_test_read_abs_y_instruction("SBC abs,Y", 0xf9);
}

bool timing_test_read_zpage_x_indirect_instructions(void)
{
    // The 7 write-to-zero-page instructions take 6 cycles.

    return
        timing_test_read_zpage_x_indirect_instruction("ORA (zpage,X)", 0x01) &&
        timing_test_read_zpage_x_indirect_instruction("AND (zpage,X)", 0x21) &&
        timing_test_read_zpage_x_indirect_instruction("EOR (zpage,X)", 0x41) &&
        timing_test_read_zpage_x_indirect_instruction("ADC (zpage,X)", 0x61) &&
        timing_test_read_zpage_x_indirect_instruction("LDA (zpage,X)", 0xa1) &&
        timing_test_read_zpage_x_indirect_instruction("CMP (zpage,X)", 0xc1) &&
        timing_test_read_zpage_x_indirect_instruction("SBC (zpage,X)", 0xe1);
}

bool timing_test_read_zpage_indirect_y_instructions(void)
{
    // The 7 write-to-zero-page instructions take 6 cycles.

    return
        timing_test_read_zpage_indirect_y_instruction("ORA (zpage),Y", 0x11) &&
        timing_test_read_zpage_indirect_y_instruction("AND (zpage),Y", 0x31) &&
        timing_test_read_zpage_indirect_y_instruction("EOR (zpage),Y", 0x51) &&
        timing_test_read_zpage_indirect_y_instruction("ADC (zpage),Y", 0x71) &&
        timing_test_read_zpage_indirect_y_instruction("LDA (zpage),Y", 0xb1) &&
        timing_test_read_zpage_indirect_y_instruction("CMP (zpage),Y", 0xd1) &&
        timing_test_read_zpage_indirect_y_instruction("SBC (zpage),Y", 0xf1);
}

bool timing_test_write_zpage_instructions(void)
{
    // The 3 write-to-zero-page instructions take 3 cycles.

    return
        timing_test_write_zpage_instruction("STA zpage", 0x85) &&
        timing_test_write_zpage_instruction("STX zpage", 0x86) &&
        timing_test_write_zpage_instruction("STY zpage", 0x84);
}

bool timing_test_write_zpage_x_instructions(void)
{
    // The 3 write-to-zero-page-with-x-indexing instructions take 4 cycles.

    return
        timing_test_write_zpage_x_instruction("STA zpage,X", 0x95) &&
        timing_test_write_zpage_x_instruction("STY zpage,X", 0x94);
}

bool timing_test_write_zpage_y_instructions(void)
{
    // The 3 write-to-zero-page-with-y-indexing instructions take 4 cycles.

    return
        timing_test_write_zpage_y_instruction("STX zpage,Y", 0x96);
}

bool timing_test_write_abs_instructions(void)
{
    // The 3 write-to-absolute-address instructions take 4 cycles.

    return
        timing_test_write_abs_instruction("STA abs", 0x8d) &&
        timing_test_write_abs_instruction("STX abs", 0x8e) &&
        timing_test_write_abs_instruction("STY abs", 0x8c);
}

bool timing_test_write_abs_x_instructions(void)
{
    // The single write-to-absolute-address-with-x-indexing instruction takes 5 cycles.

    return
        timing_test_write_abs_x_instruction("STA abs,X", 0x9d);
}

bool timing_test_write_abs_y_instructions(void)
{
    // The single write-to-absolute-address-with-y-indexing instruction takes 5 cycles.

    return
        timing_test_write_abs_y_instruction("STA abs,Y", 0x99);
}

bool timing_test_write_zpage_x_indirect_instructions(void)
{
    // The single write-to-zero-page-with-x-indexing instruction takes 6 cycles.

    return
        timing_test_write_zpage_x_indirect_instruction("STA (zpage,X)", 0x81);
}

bool timing_test_write_zpage_indirect_y_instructions(void)
{
    // The single write-to-zero-page-indirect-with-y-indexing instruction takes 6 cycles.

    return
        timing_test_write_zpage_indirect_y_instruction("STA (zpage),Y", 0x91);
}

bool timing_test_read_modify_write_zpage_instructions(void)
{
    // The 6 read-modify-write-to-zero-page instructions take 5 cycles.

    return
        timing_test_read_modify_write_zpage_instruction("ASL zpage", 0x06) &&
        timing_test_read_modify_write_zpage_instruction("ROL zpage", 0x26) &&
        timing_test_read_modify_write_zpage_instruction("LSR zpage", 0x46) &&
        timing_test_read_modify_write_zpage_instruction("ROR zpage", 0x66) &&
        timing_test_read_modify_write_zpage_instruction("DEC zpage", 0xc6) &&
        timing_test_read_modify_write_zpage_instruction("INC zpage", 0xe6);
}

bool timing_test_read_modify_write_zpage_x_instructions(void)
{
    // The 6 read-modify-write-to-zero-page-with-x-indexing instructions take 6 cycles.

    return
        timing_test_read_modify_write_zpage_x_instruction("ASL zpage,X", 0x16) &&
        timing_test_read_modify_write_zpage_x_instruction("ROL zpage,X", 0x36) &&
        timing_test_read_modify_write_zpage_x_instruction("LSR zpage,X", 0x56) &&
        timing_test_read_modify_write_zpage_x_instruction("ROR zpage,X", 0x76) &&
        timing_test_read_modify_write_zpage_x_instruction("DEC zpage,X", 0xd6) &&
        timing_test_read_modify_write_zpage_x_instruction("INC zpage,X", 0xf6);
}

bool timing_test_read_modify_write_abs_instructions(void)
{
    // The 6 read-modify-write-to-absolute-address instructions take 6 cycles.

    return
        timing_test_read_modify_write_abs_instruction("ASL abs", 0x0e) &&
        timing_test_read_modify_write_abs_instruction("ROL abs", 0x2e) &&
        timing_test_read_modify_write_abs_instruction("LSR abs", 0x4e) &&
        timing_test_read_modify_write_abs_instruction("ROR abs", 0x6e) &&
        timing_test_read_modify_write_abs_instruction("DEC abs", 0xce) &&
        timing_test_read_modify_write_abs_instruction("INC abs", 0xee);
}

bool timing_test_read_modify_write_abs_x_instructions(void)
{
    // The 6 read-modify-write-to-absolute-address-with-x-indexing instructions take 7 cycles.

    return
        timing_test_read_modify_write_abs_x_instruction("ASL abs,X", 0x1e) &&
        timing_test_read_modify_write_abs_x_instruction("ROL abs,X", 0x3e) &&
        timing_test_read_modify_write_abs_x_instruction("LSR abs,X", 0x5e) &&
        timing_test_read_modify_write_abs_x_instruction("ROR abs,X", 0x7e) &&
        timing_test_read_modify_write_abs_x_instruction("DEC abs,X", 0xde) &&
        timing_test_read_modify_write_abs_x_instruction("INC abs,X", 0xfe);
}

bool timing_test_branch_instructions(void)
{
    // The 8 branch instructions take 2 cycles if the branch is not taken,
    // and 3 or 4 cycles if the branch is taken.
    return
        timing_test_branch_instruction("BPL rel", 0x10, 0) && // Branch if N=0.
        timing_test_branch_instruction("BMI rel", 0x30, 1) && // Branch if N=1.
        timing_test_branch_instruction("BVC rel", 0x50, 0) && // Branch if V=0.
        timing_test_branch_instruction("BVS rel", 0x70, 1) && // Branch if V=1.
        timing_test_branch_instruction("BCC rel", 0x90, 0) && // Branch if C=0.
        timing_test_branch_instruction("BCS rel", 0xb0, 1) && // Branch if C=1.
        timing_test_branch_instruction("BNE rel", 0xd0, 0) && // Branch if Z=0.
        timing_test_branch_instruction("BEQ rel", 0xf0, 1); // Branch if Z=1.
}

bool timing_test_jmp_instructions(void)
{
    // The single jump-to-absolute-address instruction takes 3 cycles.
    // The single jump-to-indirect-address instruction takes 5 cycles.
    return
        timing_test_jmp_abs_instruction("JMP abs") &&
        timing_test_jmp_indirect_instruction("JMP (ind)");
}

bool timing_test_jsr_and_rts_instructions(void)
{
    // The JSR and RTS instructions both take 6 cycles.
    return
        timing_test_jsr_abs_instruction("JSR abs") &&
        timing_test_rts_instruction("RTS");
}

bool timing_test_brk_and_rti_instructions(void)
{
    // The BRK instruction takes 7 cycles; the RTI instruction takes 6 cycles.
    return
        timing_test_brk_instruction("BRK") &&
        timing_test_rti_instruction("RTI");
}

bool timing_test_6502_illegal_instructions(void)
{
    // The 6502 has 256 potential opcodes.
    //
    // 151 of these are defined, leaving 105 opcodes with undefined behavior,
    //   i.e. behavior that is not defined by the manufacturer.
    //
    // 12 of these undefined opcodes are "JAM" pseudo-instructions that jam the
    // 6502's internal state machine, thus stalling the 6502 until it is reset.
    //
    // This leaves 93 undefined opcodes with undefined behavior.
    // Even though their behavior is not defined by the manufacturer, a lot
    // is known about what happens when the 6502 encounters such opcodes.
    //
    // The NMOS 6510 Unintended Opcodes ("NoMoreSecrets") document collects all
    // known information about these opcodes. It can be downloaded from the URL
    // given below:
    //
    // https://csdb.dk/release/download.php?id=292274

    return
        //
        // Illegal SLO instruction (7 variants)
        //
        timing_test_read_modify_write_zpage_instruction           ("Illegal SLO zpage"     " (0x07)", 0x07) &&
        timing_test_read_modify_write_zpage_x_instruction         ("Illegal SLO zpage,X"   " (0x17)", 0x17) &&
        timing_test_read_modify_write_zpage_x_indirect_instruction("Illegal SLO (zpage,X)" " (0x03)", 0x03) &&
        timing_test_read_modify_write_zpage_indirect_y_instruction("Illegal SLO (zpage),Y" " (0x13)", 0x13) &&
        timing_test_read_modify_write_abs_instruction             ("Illegal SLO abs"       " (0x0f)", 0x0f) &&
        timing_test_read_modify_write_abs_x_instruction           ("Illegal SLO abs,X"     " (0x1f)", 0x1f) &&
        timing_test_read_modify_write_abs_y_instruction           ("Illegal SLO abs,Y"     " (0x1b)", 0x1b) &&
        //
        // Illegal RLA instruction (7 variants)
        //
        timing_test_read_modify_write_zpage_instruction           ("Illegal RLA zpage"     " (0x27)", 0x27) &&
        timing_test_read_modify_write_zpage_x_instruction         ("Illegal RLA zpage,X"   " (0x37)", 0x37) &&
        timing_test_read_modify_write_zpage_x_indirect_instruction("Illegal RLA (zpage,X)" " (0x23)", 0x23) &&
        timing_test_read_modify_write_zpage_indirect_y_instruction("Illegal RLA (zpage),Y" " (0x33)", 0x33) &&
        timing_test_read_modify_write_abs_instruction             ("Illegal RLA abs"       " (0x2f)", 0x2f) &&
        timing_test_read_modify_write_abs_x_instruction           ("Illegal RLA abs,X"     " (0x3f)", 0x3f) &&
        timing_test_read_modify_write_abs_y_instruction           ("Illegal RLA abs,Y"     " (0x3b)", 0x3b) &&
        //
        // Illegal SRE instruction (7 variants)
        //
        timing_test_read_modify_write_zpage_instruction           ("Illegal SRE zpage"     " (0x47)", 0x47) &&
        timing_test_read_modify_write_zpage_x_instruction         ("Illegal SRE zpage,X"   " (0x57)", 0x57) &&
        timing_test_read_modify_write_zpage_x_indirect_instruction("Illegal SRE (zpage,X)" " (0x43)", 0x43) &&
        timing_test_read_modify_write_zpage_indirect_y_instruction("Illegal SRE (zpage),Y" " (0x53)", 0x53) &&
        timing_test_read_modify_write_abs_instruction             ("Illegal SRE abs"       " (0x4f)", 0x4f) &&
        timing_test_read_modify_write_abs_x_instruction           ("Illegal SRE abs,X"     " (0x5f)", 0x5f) &&
        timing_test_read_modify_write_abs_y_instruction           ("Illegal SRE abs,Y"     " (0x5b)", 0x5b) &&
        //
        // Illegal RRA instruction (7 variants)
        //
        timing_test_read_modify_write_zpage_instruction           ("Illegal RRA zpage"     " (0x67)", 0x67) &&
        timing_test_read_modify_write_zpage_x_instruction         ("Illegal RRA zpage,X"   " (0x77)", 0x77) &&
        timing_test_read_modify_write_zpage_x_indirect_instruction("Illegal RRA (zpage,X)" " (0x63)", 0x63) &&
        timing_test_read_modify_write_zpage_indirect_y_instruction("Illegal RRA (zpage),Y" " (0x73)", 0x73) &&
        timing_test_read_modify_write_abs_instruction             ("Illegal RRA abs"       " (0x6f)", 0x6f) &&
        timing_test_read_modify_write_abs_x_instruction           ("Illegal RRA abs,X"     " (0x7f)", 0x7f) &&
        timing_test_read_modify_write_abs_y_instruction           ("Illegal RRA abs,Y"     " (0x7b)", 0x7b) &&
        //
        // Illegal SAX instruction (4 variants)
        //
        timing_test_write_zpage_instruction           ("Illegal SAX zpage"     " (0x87)", 0x87) &&
        timing_test_write_zpage_y_instruction         ("Illegal SAX zpage,Y"   " (0x97)", 0x97) &&
        timing_test_write_zpage_x_indirect_instruction("Illegal SAX (zpage,X)" " (0x83)", 0x83) &&
        timing_test_write_abs_instruction             ("Illegal SAX abs"       " (0x8f)", 0x8f) &&
        //
        // Illegal LAX instruction (6 variants)
        //
        timing_test_read_zpage_instruction            ("Illegal LAX zpage"     " (0xa7)", 0xa7) &&
        timing_test_read_zpage_y_instruction          ("Illegal LAX zpage,Y"   " (0xb7)", 0xb7) &&
        timing_test_read_zpage_x_indirect_instruction ("Illegal LAX (zpage,X)" " (0xa3)", 0xa3) &&
        timing_test_read_zpage_indirect_y_instruction ("Illegal LAX (zpage),Y" " (0xb3)", 0xb3) &&
        timing_test_read_abs_instruction              ("Illegal LAX abs"       " (0xaf)", 0xaf) &&
        timing_test_read_abs_y_instruction            ("Illegal LAX abs,Y"     " (0xbf)", 0xbf) &&
        //
        // Illegal DCP instruction (7 variants)
        //
        timing_test_read_modify_write_zpage_instruction            ("Illegal DCP zpage"     " (0xc7)", 0xc7) &&
        timing_test_read_modify_write_zpage_x_instruction          ("Illegal DCP zpage,X"   " (0xd7)", 0xd7) &&
        timing_test_read_modify_write_zpage_x_indirect_instruction ("Illegal DCP (zpage,X)" " (0xc3)", 0xc3) &&
        timing_test_read_modify_write_zpage_indirect_y_instruction ("Illegal DCP (zpage),Y" " (0xd3)", 0xd3) &&
        timing_test_read_modify_write_abs_instruction              ("Illegal DCP abs"       " (0xcf)", 0xcf) &&
        timing_test_read_modify_write_abs_x_instruction            ("Illegal DCP abs,X"     " (0xdf)", 0xdf) &&
        timing_test_read_modify_write_abs_y_instruction            ("Illegal DCP abs,Y"     " (0xdb)", 0xdb) &&
        //
        // Illegal ISC instruction (7 variants)
        //
        timing_test_read_modify_write_zpage_instruction            ("Illegal ISC zpage"     " (0xe7)", 0xe7) &&
        timing_test_read_modify_write_zpage_x_instruction          ("Illegal ISC zpage,X"   " (0xf7)", 0xf7) &&
        timing_test_read_modify_write_zpage_x_indirect_instruction ("Illegal ISC (zpage,X)" " (0xe3)", 0xe3) &&
        timing_test_read_modify_write_zpage_indirect_y_instruction ("Illegal ISC (zpage),Y" " (0xf3)", 0xf3) &&
        timing_test_read_modify_write_abs_instruction              ("Illegal ISC abs"       " (0xef)", 0xef) &&
        timing_test_read_modify_write_abs_x_instruction            ("Illegal ISC abs,X"     " (0xff)", 0xff) &&
        timing_test_read_modify_write_abs_y_instruction            ("Illegal ISC abs,Y"     " (0xfb)", 0xfb) &&
        //
        // Illegal ANC instruction (2 variants)
        //
        timing_test_read_immediate_instruction("Illegal ANC #imm" " (0x0b)", 0x0b) &&
        timing_test_read_immediate_instruction("Illegal ANC #imm" " (0x2b)", 0x2b) &&
        //
        // Illegal ALR instruction (1 variant)
        //
        timing_test_read_immediate_instruction("Illegal ALR #imm" " (0x4b)", 0x4b) &&
        //
        // Illegal ARR instruction (1 variant)
        //
        timing_test_read_immediate_instruction("Illegal ARR #imm" " (0x6b)", 0x6b) &&
        //
        // Illegal SBX instruction (1 variant)
        //
        timing_test_read_immediate_instruction("Illegal SBX #imm" " (0xcb)", 0xcb) &&
        //
        // Illegal SBC instruction (1 variant) -- this is an undocument instruction equivalent to the SBC instruction.
        //
        timing_test_read_immediate_instruction("Illegal SBC #imm" " (0xeb)", 0xeb) &&
        //
        // Illegal LAS instruction (1 variant)
        //
        timing_test_read_abs_y_instruction_save_sp("Illegal LAS abs,Y" " (0xbb)", 0xbb) &&
        //
        // Illegal NOP instruction (27 variants)
        //
        timing_test_single_byte_instruction_sequence("Illegal NOP" " (0x1a)", 0x1a, 2) &&
        timing_test_single_byte_instruction_sequence("Illegal NOP" " (0x3a)", 0x3a, 2) &&
        timing_test_single_byte_instruction_sequence("Illegal NOP" " (0x5a)", 0x5a, 2) &&
        timing_test_single_byte_instruction_sequence("Illegal NOP" " (0x7a)", 0x7a, 2) &&
        timing_test_single_byte_instruction_sequence("Illegal NOP" " (0xda)", 0xda, 2) &&
        timing_test_single_byte_instruction_sequence("Illegal NOP" " (0xfa)", 0xfa, 2) &&
        //
        timing_test_read_immediate_instruction("Illegal NOP #imm"  " (0x80)", 0x80) &&
        timing_test_read_immediate_instruction("Illegal NOP #imm"  " (0x82)", 0x82) &&
        timing_test_read_immediate_instruction("Illegal NOP #imm"  " (0x89)", 0x89) &&
        timing_test_read_immediate_instruction("Illegal NOP #imm"  " (0xc2)", 0xc2) &&
        timing_test_read_immediate_instruction("Illegal NOP #imm"  " (0xe2)", 0xe2) &&
        //
        timing_test_read_zpage_instruction("Illegal NOP zpage"     " (0x04)", 0x04) &&
        timing_test_read_zpage_instruction("Illegal NOP zpage"     " (0x44)", 0x44) &&
        timing_test_read_zpage_instruction("Illegal NOP zpage"     " (0x64)", 0x64) &&
        //
        timing_test_read_zpage_x_instruction("Illegal NOP zpage,X" " (0x14)", 0x14) &&
        timing_test_read_zpage_x_instruction("Illegal NOP zpage,X" " (0x34)", 0x34) &&
        timing_test_read_zpage_x_instruction("Illegal NOP zpage,X" " (0x54)", 0x54) &&
        timing_test_read_zpage_x_instruction("Illegal NOP zpage,X" " (0x74)", 0x74) &&
        timing_test_read_zpage_x_instruction("Illegal NOP zpage,X" " (0xd4)", 0xd4) &&
        timing_test_read_zpage_x_instruction("Illegal NOP zpage,X" " (0xf4)", 0xf4) &&
        //
        timing_test_read_abs_instruction("Illegal NOP abs"         " (0x0c)", 0x0c) &&
        //
        timing_test_read_abs_x_instruction("Illegal NOP abs,X"     " (0x1c)", 0x1c) &&
        timing_test_read_abs_x_instruction("Illegal NOP abs,X"     " (0x3c)", 0x3c) &&
        timing_test_read_abs_x_instruction("Illegal NOP abs,X"     " (0x5c)", 0x5c) &&
        timing_test_read_abs_x_instruction("Illegal NOP abs,X"     " (0x7c)", 0x7c) &&
        timing_test_read_abs_x_instruction("Illegal NOP abs,X"     " (0xdc)", 0xdc) &&
        timing_test_read_abs_x_instruction("Illegal NOP abs,X"     " (0xfc)", 0xfc) &&

        // Illegal JAM instruction (12 variants). These are not tested.
        //
        //     0x02, 0x12, 0x22, 0x32, 0x42, 0x54, 0x62, 0x72, 0x92, 0xb2, 0xd2, 0xf2.

        // *** SHA/SHX/SHY instructions ***
        //
        // Testing the SHA/SHX/SHY instructions somehow triggers undefined in the test program.
        // This happens both on the Altirra simulator and on a real Atari.
        // The crash is not currently understood.
        //
        // This could either be caused by a real side-effect of these undocumented instructions,
        // or it could indicate a bug in the test program.
        //
        // If we replace the SHA/SHX/SHY opcodes with documented opcodes that should behave
        // the same in terms of memory access and timing, the crash behavior is not seen.
        //
        // For now, we skip these four opcodes. If you want to test/debug them, use the
        // 'timing_test_buggy_6502_illegal_instructions' routine defined below.
        //
        // Illegal SHA instruction (2 variants)
        //
        // timing_test_write_zpage_indirect_y_instruction("Illegal SHA (zpage),Y" " (0x93)", 0x93) &&
        // timing_test_write_abs_y_instruction           ("Illegal SHA abs,Y"     " (0x9f)", 0x9f) &&
        //
        // Illegal SHX instruction (1 variant)
        //
        // timing_test_write_abs_y_instruction           ("Illegal SHX abs,Y"     " (0x9e)", 0x9e) &&
        //
        // Illegal SHY instruction (1 variant)
        //
        // timing_test_write_abs_x_instruction           ("Illegal SHY abs,X"     " (0x9c)", 0x9c) &&

        // Illegal TAS instruction (1 variant).
        //
        timing_test_write_abs_y_instruction_save_sp   ("Illegal TAS abs,Y"     " (0x9b)", 0x9b) &&
        //
        // Illegal ANE instruction (1 variant)
        //
        timing_test_read_immediate_instruction        ("Illegal ANE #imm"      " (0x8b)", 0x8b) &&
        //
        // Illegal LAX instruction (1 variant)
        //
        timing_test_read_immediate_instruction        ("Illegal LAX #imm"      " (0xab)", 0xab);
}

bool timing_test_buggy_6502_illegal_instructions(void)
{
    // *** SHA/SHX/SHY instructions ***
    //
    // Testing the SHA/SHX/SHY instructions somehow triggers undefined in the test program.
    // This happens both on the Altirra simulator and on a real Atari.
    // The crash is not currently understood.
    //
    // This could either be caused by a real side-effect of these undocumented instructions,
    // or it could indicate a bug in the test program.
    //
    // If we replace the SHA/SHX/SHY opcodes with documented opcodes that should behave
    // the same in terms of memory access and timing, the crash behavior is not seen.
    //
    // Replacements used:
    //
    //    STA (zpage,Y)    0x91   instead of what we want to test:     SHA (zpage,Y)    0x93
    //    STA abs,Y        0x99   instead of what we want to test:     SHA abs,Y        0x9f
    //    STA (zpage,Y)    0x99   instead of what we want to test:     SHX abs,Y        0x9e
    //    STA abs,X        0x9d   instead of what we want to test:     SHY abs,X        0x9c

    return
        timing_test_write_zpage_indirect_y_instruction("Illegal SHA (zpage),Y" " (0x93)", 0x93) &&  // Should be 0x93 to test SHA.
        timing_test_write_abs_y_instruction           ("Illegal SHA abs,Y"     " (0x9f)", 0x9f) &&  // Should be 0x9f to test SHA.
        timing_test_write_abs_y_instruction           ("Illegal SHX abs,Y"     " (0x9e)", 0x9e) &&  // Should be 0x9e to test SHX.
        timing_test_write_abs_x_instruction           ("Illegal SHY abs,X"     " (0x9c)", 0x9c);    // Should be 0x9c to test SHY.
}

bool run_6502_instruction_timing_tests(void)
{
    return
        // Test the timing of the 2 single-byte stack-pointer transfer instructions.
        timing_test_stackpointer_transfer_instructions() &&
        // Test the timing of the 4 single-byte stack push/pull instructions.
        timing_test_single_byte_stack_push_pull_instructions() &&
        // Test the timing of the 20 single-byte, two-cycle opcodes.
        timing_test_single_byte_two_cycle_implied_instructions() &&
        // Test the timing of the 11 two-byte read-immediate instructions.
        timing_test_read_immediate_instructions() &&
        // Test the timing of the 12 two-byte read-from-zero-page instructions.
        timing_test_read_zpage_instructions() &&
        // Test the timing of the 8 two-byte read-from-zero-page-with-x-indexing instructions.
        timing_test_read_zpage_x_instructions() &&
        // Test the timing of the 1 two-byte read-from-zero-page-with-y-indexing instruction.
        timing_test_read_zpage_y_instructions() &&
        // Test the timing of the 12 two-byte read-from-abs-address instructions.
        timing_test_read_abs_instructions() &&
        // Test the timing of the 8 three-byte read-from-zero-page-with-x-indexing instructions.
        timing_test_read_abs_x_instructions() &&
        // Test the timing of the 8 three-byte read-from-zero-page-with-y-indexing instructions.
        timing_test_read_abs_y_instructions() &&
        // Test the timing of the 7 two-byte read-zpage_with-x-indexing-indirect instructions.
        timing_test_read_zpage_x_indirect_instructions() &&
        // Test the timing of the 7 two-byte read-zpage_indirect_with-y-indexing instructions.
        timing_test_read_zpage_indirect_y_instructions() &&
        // Test the timing of the 3 two-byte write-to-zero-page instructions.
        timing_test_write_zpage_instructions() &&
        // Test the timing of the 2 two-byte write-to-zero-page-with-x-indexing instructions.
        timing_test_write_zpage_x_instructions() &&
        // Test the timing of the 1 two-byte write-to-zero-page-with-y-indexing instruction.
        timing_test_write_zpage_y_instructions() &&
        // Test the timing of the 3 three-byte write-to-absolute-address instructions.
        timing_test_write_abs_instructions() &&
        // Test the timing of the 1 three-byte write-to-absolute-address-with-x-indexing instruction.
        timing_test_write_abs_x_instructions() &&
        // Test the timing of the 1 three-byte write-to-absolute-address-with-y-indexing instruction.
        timing_test_write_abs_y_instructions() &&
        // Test the timing of the 7 two-byte read-zpage_with-x-indexing-indirect instructions.
        timing_test_write_zpage_x_indirect_instructions() &&
        // Test the timing of the 7 two-byte read-zpage_indirect_with-y-indexing instructions.
        timing_test_write_zpage_indirect_y_instructions() &&
        // Test the timing of the 24 read-modify-write instructions.
        timing_test_read_modify_write_zpage_instructions()   && // 6 instructions
        timing_test_read_modify_write_zpage_x_instructions() && // 6 instructions
        timing_test_read_modify_write_abs_instructions()     && // 6 instructions
        timing_test_read_modify_write_abs_x_instructions()   && // 6 instructions
        // Test the timing of the 14 branch, jump, jsr, and interrupt related instructions.
        timing_test_branch_instructions()                    && // 8 instructions.
        timing_test_jmp_instructions()                       && // 2 instructions.
        timing_test_jsr_and_rts_instructions()               && // 2 instructions.
        timing_test_brk_and_rti_instructions()               && // 2 instructions.
        // Test 93 of the 105 "illegal" instructions (all excluding the 12 JAM instructions).
        timing_test_6502_illegal_instructions();
}


void tic_cmd_cpu_test(unsigned level)
{
    const unsigned lookup_table[8] = {1, 3, 5, 15, 17, 51, 85, 255};
    bool run_completed;

    if (level > 7)
    {
        level = 7;
    }

    STEP_SIZE = lookup_table[7 - level];

    reset_test_counts();
    pre_big_measurement_block_hook();

    run_completed = run_6502_instruction_timing_tests();
    //run_completed = timing_test_buggy_6502_illegal_instructions();

    post_big_measurement_block_hook();
    report_test_counts();

    if (run_completed)
    {
        printf("ALL TESTS PASSED.\n");
    }
    else
    {
        if (error_count == 0)
        {
            printf("TEST STOPPED BY USER REQUEST.\n");
        }
        else
        {
            printf("TEST STOPPED DUE TO ERROR.\n");
        }
    }

    printf("\n");
}
