
////////////////////////////
// timing_test_routines.c //
////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "timing_test_memory.h"
#include "timing_test_measurement.h"
#include "target.h"

unsigned STEP_SIZE = 85;
unsigned LAST = 255;

#define DEFAULT_RUN_FLAGS (F_STOP_ON_ERROR)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                                 ZPAGE PRESERVATION                                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t num_zpage_preserve; // How many zero-pages addresses should the test preserve?
uint8_t zpage_preserve[2];  // Zero page addresses to preserve while the test executes (0, 1, or 2 values).

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                              TRIVIAL SUPPORT ROUTINES                                             //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t lsb(uint8_t * ptr)
{
    return (uint16_t)ptr & 0xff;
}

static uint8_t msb(uint8_t * ptr)
{
    return (uint16_t)ptr >> 8;
}

static bool different_pages(uint8_t * u1, uint8_t * u2)
{
    return msb(u1) != msb(u2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                          PREPROCESSOR SYMBOLS FOR 6502 OPCODES                                    //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OPC_BRK  0x00
#define OPC_PHP  0x08
#define ORA_IMM  0x09
#define JSR_ABS  0x20
#define OPC_PLP  0x28
#define AND_IMM  0x29
#define OPC_RTI  0x40
#define OPC_PHA  0x48
#define JMP_ABS  0x4c
#define OPC_RTS  0x60
#define OPC_PLA  0x68
#define JMP_IND  0x6c
#define STY_ZP   0x84
#define STX_ZP   0x86
#define STX_ABS  0x8e
#define OPC_TXS  0x9a
#define LDY_IMM  0xa0
#define LDX_IMM  0xa2
#define LDA_IMM  0xa9
#define LDX_ABS  0xae
#define OPC_TSX  0xba

uint8_t par1;
uint8_t par2;
uint8_t par3;
uint8_t par4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                           TIMING TESTS FOR SINGLE, DOUBLE, AND TRIPLE BYTE INSTRUCTION SEQUENCES                  //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_single_byte_instruction_sequence(const char * test_description, uint8_t opc, unsigned instruction_cycles)
{
    // LOOPS: opcode_offset

    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0] = opc;     // OPC
        opcode_address[1] = OPC_RTS; // RTS [-]

        if (!run_measurement(test_description, 0, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par1))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_two_byte_instruction_sequence(const char * test_description, uint8_t opc1, uint8_t opc2, unsigned test_overhead_cycles, unsigned instruction_cycles)
{
    // LOOPS: opcode_offset

    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0] = opc1;    // OPC
        opcode_address[1] = opc2;    // OPC
        opcode_address[2] = OPC_RTS; // RTS [-]

        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par1))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_three_byte_instruction_sequence(const char * test_description, uint8_t opc1, uint8_t opc2, uint8_t opc3, unsigned test_overhead_cycles, unsigned instruction_cycles)
{
    // LOOPS: opcode_offset

    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0] = opc1;    // OPC
        opcode_address[1] = opc2;    // OPC
        opcode_address[2] = opc3;    // OPC
        opcode_address[3] = OPC_RTS; // RTS [-]

        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par1))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                         TIMING TESTS FOR READ INSTRUCTIONS                                        //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_read_immediate_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, operand

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            opcode_address[0] = opcode;  // OPC #par2 [2]
            opcode_address[1] = par2;    //
            opcode_address[2] = OPC_RTS; // RTS       [-]

            test_overhead_cycles = 0;
            instruction_cycles = 2;

            if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par12))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            if (zp_address_is_safe(par2))
            {
                opcode_address[0] = opcode;     // OPC par2        [3]
                opcode_address[1] = par2;       //
                opcode_address[2] = OPC_RTS;    // RTS             [-]

                test_overhead_cycles = 0;
                instruction_cycles = 3;

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par12))
                    return false;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_x

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe(par2 + par3))
                {
                    opcode_address[-2] = LDX_IMM; // LDX #par3        [2]
                    opcode_address[-1] = par3;    //
                    opcode_address[ 0] = opcode;  // OPC par2,X       [4]
                    opcode_address[ 1] = par2;    //
                    opcode_address[ 2] = OPC_RTS; // RTS              [-]

                    test_overhead_cycles = 2;
                    instruction_cycles = 4;

                    if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_y

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe(par2 + par3))
                {
                    opcode_address[-2] = LDY_IMM;    // LDY #par3          [2]
                    opcode_address[-1] = par3;       //
                    opcode_address[ 0] = opcode;     // OPC par2,Y         [4]
                    opcode_address[ 1] = par2;       //
                    opcode_address[ 2] = OPC_RTS;    // RTS                [-]

                    test_overhead_cycles = 2;
                    instruction_cycles = 4;

                    if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_abs_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * read_address = TESTCODE_BASE + par2;

            opcode_address[0] = opcode;            // OPC read_address   [4]
            opcode_address[1] = lsb(read_address); //
            opcode_address[2] = msb(read_address); //
            opcode_address[3] = OPC_RTS;           // RTS                [-]

            test_overhead_cycles = 0;
            instruction_cycles = 4;

            if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par12))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_abs_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_x

    unsigned instruction_cycles, test_overhead_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = LDX_IMM;           // LDX #par3            [2]
                opcode_address[-1] = par3;              //
                opcode_address[ 0] = opcode;            // OPC base_address,X   [4 or 5]
                opcode_address[ 1] = lsb(base_address); //
                opcode_address[ 2] = msb(base_address); //
                opcode_address[ 3] = OPC_RTS;           // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 4 + different_pages(base_address, base_address + par3);

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_abs_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_y

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = LDY_IMM;           // LDY #par3            [2]
                opcode_address[-1] = par3;             //
                opcode_address[ 0] = opcode;            // OPC base_address,Y   [4 or 5]
                opcode_address[ 1] = lsb(base_address); //
                opcode_address[ 2] = msb(base_address); //
                opcode_address[ 3] = OPC_RTS;           // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 4 + different_pages(base_address, base_address + par3);

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}


bool timing_test_read_abs_y_instruction_save_sp(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_y

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-6] = OPC_TSX;                 // TSX          [2]
                opcode_address[-5] = STX_ABS;                 // STX save_sp  [4]
                opcode_address[-4] = lsb(opcode_address + 8); //
                opcode_address[-3] = msb(opcode_address + 8); //
                opcode_address[-2] = LDY_IMM;                 // LDY #par3     2]
                opcode_address[-1] = par3;                    //
                opcode_address[ 0] = opcode;                  // OPC base_address,Y   [4 or 5]
                opcode_address[ 1] = lsb(base_address);       //
                opcode_address[ 2] = msb(base_address);       //
                opcode_address[ 3] = LDX_ABS;                 // LDX save_sp  [4]
                opcode_address[ 4] = lsb(opcode_address + 8); //
                opcode_address[ 5] = msb(opcode_address + 8); //
                opcode_address[ 6] = OPC_TXS;                 // TXS          [2]
                opcode_address[ 7] = OPC_RTS;                 // RTS          [-]

                test_overhead_cycles = 2 + 4 + 2 + 4 + 2;
                instruction_cycles = 4 + different_pages(base_address, base_address + par3);

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 6, DEFAULT_RUN_FLAGS, Par123))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_x_indirect_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_base_address, reg_x, address_offset

    unsigned zp_ptr_lo_address;
    unsigned zp_ptr_hi_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                // (zp_ptr_lo, zp_ptr_hi) will be the actual pointer used for indirection.
                zp_ptr_lo_address = (par2 + par3 + 0) & 0xff;
                zp_ptr_hi_address = (par2 + par3 + 1) & 0xff;

                if (zp_address_is_safe(zp_ptr_lo_address) && zp_address_is_safe(zp_ptr_hi_address))
                {
                    zpage_preserve[0] = zp_ptr_lo_address;
                    zpage_preserve[1] = zp_ptr_hi_address;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        uint8_t * abs_address = TESTCODE_BASE + par4;

                        opcode_address[-10] = LDX_IMM;           // LDX #<abs_address     [2]
                        opcode_address[ -9] = lsb(abs_address);  //
                        opcode_address[ -8] = STX_ZP;            // STX zp_ptr_lo_address [3]
                        opcode_address[ -7] = zp_ptr_lo_address; //
                        opcode_address[ -6] = LDX_IMM;           // LDX #>abs_address     [2]
                        opcode_address[ -5] = msb(abs_address);  //
                        opcode_address[ -4] = STX_ZP;            // STX zp_ptr_hi_address [3]
                        opcode_address[ -3] = zp_ptr_hi_address; //
                        opcode_address[ -2] = LDX_IMM;           // LDX #par3             [2]
                        opcode_address[ -1] = par3;              //
                        opcode_address[  0] = opcode;            // OPC (zpage, X)        [6]
                        opcode_address[  1] = par2;              //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        instruction_cycles = 6;

                        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 10, DEFAULT_RUN_FLAGS, Par1234))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_zpage_indirect_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_ptr_lo_address, address_offset, reg_y

    unsigned zp_ptr_lo_address;
    unsigned zp_ptr_hi_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            zp_ptr_lo_address = par2;
            zp_ptr_hi_address = (zp_ptr_lo_address + 1) & 0xff;

            if (zp_address_is_safe(zp_ptr_lo_address) && zp_address_is_safe(zp_ptr_hi_address))
            {
                zpage_preserve[0] = zp_ptr_lo_address;
                zpage_preserve[1] = zp_ptr_hi_address;

                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    uint8_t * base_address = TESTCODE_BASE + par3;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        uint8_t * effective_address = base_address + par4;

                        opcode_address[-10] = LDY_IMM;           // LDY #<base_address    [2]
                        opcode_address[ -9] = lsb(base_address); //
                        opcode_address[ -8] = STY_ZP;            // STY zp_ptr_lo_address [3]
                        opcode_address[ -7] = zp_ptr_lo_address; //
                        opcode_address[ -6] = LDY_IMM;           // LDY #>base_address    [2]
                        opcode_address[ -5] = msb(base_address); //
                        opcode_address[ -4] = STY_ZP;            // STY zp_ptr_hi_address [3]
                        opcode_address[ -3] = zp_ptr_hi_address; //
                        opcode_address[ -2] = LDY_IMM;           // LDY #par4             [2]
                        opcode_address[ -1] = par4;             //
                        opcode_address[  0] = opcode;            // OPC (zpage), Y        [5 or 6]
                        opcode_address[  1] = zp_ptr_lo_address; //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        instruction_cycles = 5 + different_pages(base_address, effective_address);

                        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 10, DEFAULT_RUN_FLAGS, Par1234))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                    if (par3 == LAST)
                        break;
                }
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                        TIMING TESTS FOR WRITE INSTRUCTIONS                                        //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_write_zpage_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            if (zp_address_is_safe(par2))
            {
                zpage_preserve[0] = par2;

                opcode_address[0] = opcode;  // OPC par2   [3]
                opcode_address[1] = par2;    //
                opcode_address[2] = OPC_RTS; // RTS        [-]

                test_overhead_cycles = 0;
                instruction_cycles = 3;

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par12))
                    return false;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_zpage_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_x

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe(par2 + par3))
                {
                    zpage_preserve[0] = par2 + par3;

                    opcode_address[-2] = LDX_IMM; // LDX #imm       [2]
                    opcode_address[-1] = par3;   //
                    opcode_address[ 0] = opcode;  // OPC par2,X     [4]
                    opcode_address[ 1] = par2;    //
                    opcode_address[ 2] = OPC_RTS; // RTS            [1]

                    test_overhead_cycles = 2;
                    instruction_cycles = 4;

                    if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_zpage_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_y

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe(par2 + par3))
                {
                    zpage_preserve[0] = par2 + par3;

                    opcode_address[-2] = LDY_IMM; // LDY #imm    [2]
                    opcode_address[-1] = par3;    //
                    opcode_address[ 0] = opcode;  // OPC par2,Y  [4]
                    opcode_address[ 1] = par2;    //
                    opcode_address[ 2] = OPC_RTS; // RTS         [-]

                    test_overhead_cycles = 2;
                    instruction_cycles = 4;

                    if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_abs_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * write_address = TESTCODE_BASE + par2;

            opcode_address[0] = opcode;             // OPC write_address   [4]
            opcode_address[1] = lsb(write_address); //
            opcode_address[2] = msb(write_address); //
            opcode_address[3] = OPC_RTS;            // RTS                 [-]

            test_overhead_cycles = 0;
            instruction_cycles = 4;

            if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par12))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_abs_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_x

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = LDX_IMM;           // LDX #par3            [2]
                opcode_address[-1] = par3;              //
                opcode_address[ 0] = opcode;            // OPC base_address,X   [5]
                opcode_address[ 1] = lsb(base_address); //
                opcode_address[ 2] = msb(base_address); //
                opcode_address[ 3] = OPC_RTS;           // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 5;

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_abs_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_y

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = LDY_IMM;           // LDY #par3            [2]
                opcode_address[-1] = par3;              //
                opcode_address[ 0] = opcode;            // OPC base_address,Y   [5]
                opcode_address[ 1] = lsb(base_address); //
                opcode_address[ 2] = msb(base_address); //
                opcode_address[ 3] = OPC_RTS;           // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 5;

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_abs_y_instruction_save_sp(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_y

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-6] = OPC_TSX;                 // TSX          [2]
                opcode_address[-5] = STX_ABS;                 // STX save_sp  [4]
                opcode_address[-4] = lsb(opcode_address + 8); //
                opcode_address[-3] = msb(opcode_address + 8); //
                opcode_address[-2] = LDY_IMM;                 // LDY #imm     [2]
                opcode_address[-1] = par3;                    //
                opcode_address[ 0] = opcode;                  // OPC base_address,Y   [5]
                opcode_address[ 1] = lsb(base_address);       //
                opcode_address[ 2] = msb(base_address);       //
                opcode_address[ 3] = LDX_ABS;                 // LDX save_sp  [4]
                opcode_address[ 4] = lsb(opcode_address + 8); //
                opcode_address[ 5] = msb(opcode_address + 8); //
                opcode_address[ 6] = OPC_TXS;                 // TXS          [2]
                opcode_address[ 7] = OPC_RTS;                 // RTS          [-]

                test_overhead_cycles = 2 + 4 + 2 + 4 + 2;
                instruction_cycles = 5;

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 6, DEFAULT_RUN_FLAGS, Par123))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_zpage_x_indirect_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_base_address, reg_x, address_offset

    unsigned zp_ptr_lo_address;
    unsigned zp_ptr_hi_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                // (zp_ptr_lo, zp_ptr_hi) will be the actual pointer used for indirection.
                zp_ptr_lo_address = (par2 + par3 + 0) & 0xff;
                zp_ptr_hi_address = (par2 + par3 + 1) & 0xff;

                zpage_preserve[0] = zp_ptr_lo_address;
                zpage_preserve[1] = zp_ptr_hi_address;

                if (zp_address_is_safe(zp_ptr_lo_address) && zp_address_is_safe(zp_ptr_hi_address))
                {
                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        uint8_t * abs_address = TESTCODE_BASE + par4;

                        opcode_address[-10] = LDX_IMM;           // LDX #<abs_address     [2]
                        opcode_address[ -9] = lsb(abs_address);  //
                        opcode_address[ -8] = STX_ZP;            // STX zp_ptr_lo_address [3]
                        opcode_address[ -7] = zp_ptr_lo_address; //
                        opcode_address[ -6] = LDX_IMM;           // LDX #>abs_address     [2]
                        opcode_address[ -5] = msb(abs_address);  //
                        opcode_address[ -4] = STX_ZP;            // STX zp_ptr_hi_address [3]
                        opcode_address[ -3] = zp_ptr_hi_address; //
                        opcode_address[ -2] = LDX_IMM;           // LDX #par3             [2]
                        opcode_address[ -1] = par3;             //
                        opcode_address[  0] = opcode;            // OPC (zpage, X)        [6]
                        opcode_address[  1] = par2;              //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        instruction_cycles = 6;

                        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 10, DEFAULT_RUN_FLAGS, Par1234))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_write_zpage_indirect_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_ptr_lo_address, address_offset, reg_y

    unsigned zp_ptr_lo_address;
    unsigned zp_ptr_hi_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            zp_ptr_lo_address = par2;
            zp_ptr_hi_address = (zp_ptr_lo_address + 1) & 0xff;

            zpage_preserve[0] = zp_ptr_lo_address;
            zpage_preserve[1] = zp_ptr_hi_address;

            if (zp_address_is_safe(zp_ptr_lo_address) && zp_address_is_safe(zp_ptr_hi_address))
            {
                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    uint8_t * base_address = TESTCODE_BASE + par3;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        uint8_t * effective_address = base_address + par4;

                        opcode_address[-10] = LDY_IMM;           // LDY #<base_address    [2]
                        opcode_address[ -9] = lsb(base_address); //
                        opcode_address[ -8] = STY_ZP;            // STY zp_ptr_lo_address [3]
                        opcode_address[ -7] = zp_ptr_lo_address; //
                        opcode_address[ -6] = LDY_IMM;           // LDY #>base_address    [2]
                        opcode_address[ -5] = msb(base_address); //
                        opcode_address[ -4] = STY_ZP;            // STY zp_ptr_hi_address [3]
                        opcode_address[ -3] = zp_ptr_hi_address; //
                        opcode_address[ -2] = LDY_IMM;           // LDY #par4             [2]
                        opcode_address[ -1] = par4;             //
                        opcode_address[  0] = opcode;            // OPC (zpage), Y        [5 or 6]
                        opcode_address[  1] = zp_ptr_lo_address; //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        instruction_cycles = 6;

                        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 10, DEFAULT_RUN_FLAGS, Par1234))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                    if (par3 == LAST)
                        break;
                }
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                                    TIMING TESTS FOR READ-MODIFY-WRITE INSTRUCTIONS                                //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_read_modify_write_zpage_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            if (zp_address_is_safe(par2))
            {
                zpage_preserve[0] = par2;

                opcode_address[0] = opcode;  // OPC par2    [5]
                opcode_address[1] = par2;    //
                opcode_address[2] = OPC_RTS; // RTS         [-]

                test_overhead_cycles = 0;
                instruction_cycles = 5;

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par12))
                    return false;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_zpage_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_x

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 1; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                if (zp_address_is_safe(par2 + par3))
                {
                    zpage_preserve[0] = par2 + par3;

                    opcode_address[-2] = LDX_IMM; // LDX #par3     [2]
                    opcode_address[-1] = par3;    //
                    opcode_address[ 0] = opcode;  // OPC par2,X    [6]
                    opcode_address[ 1] = par2;    //
                    opcode_address[ 2] = OPC_RTS; // RTS           [-]

                    test_overhead_cycles = 2;
                    instruction_cycles = 6;

                    if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                        return false;
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_abs_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * abs_address = TESTCODE_BASE + par2;

            opcode_address[0] = opcode;           // OPC abs_address    [6]
            opcode_address[1] = lsb(abs_address); //
            opcode_address[2] = msb(abs_address); //
            opcode_address[3] = OPC_RTS;          // RTS                [-]

            test_overhead_cycles = 0;
            instruction_cycles = 6;

            if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par12))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_abs_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_x

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = LDX_IMM;           // LDX #par3            [2]
                opcode_address[-1] = par3;              //
                opcode_address[ 0] = opcode;            // OPC base_address,X   [7]
                opcode_address[ 1] = lsb(base_address); //
                opcode_address[ 2] = msb(base_address); //
                opcode_address[ 3] = OPC_RTS;           // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 7;

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_abs_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_y

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + par2;

            for (par3 = 0;;par3 += STEP_SIZE)
            {
                opcode_address[-2] = LDY_IMM;           // LDY #par3            [2]
                opcode_address[-1] = par3;              //
                opcode_address[ 0] = opcode;            // OPC base_address,Y   [7]
                opcode_address[ 1] = lsb(base_address); //
                opcode_address[ 2] = msb(base_address); //
                opcode_address[ 3] = OPC_RTS;           // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 7;

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS, Par123))
                    return false;

                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_zpage_x_indirect_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_base_address, reg_x, address_offset

    unsigned zp_ptr_lo_address;
    unsigned zp_ptr_hi_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            for (par3 = 0;;par3 += STEP_SIZE)
            {
                // (zp_ptr_lo, zp_ptr_hi) will be the actual pointer used for indirection.
                zp_ptr_lo_address = (par2 + par3 + 0) & 0xff;
                zp_ptr_hi_address = (par2 + par3 + 1) & 0xff;

                zpage_preserve[0] = zp_ptr_lo_address;
                zpage_preserve[1] = zp_ptr_hi_address;

                if (zp_address_is_safe(zp_ptr_lo_address) && zp_address_is_safe(zp_ptr_hi_address))
                {
                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        uint8_t * abs_address = TESTCODE_BASE + par4;

                        opcode_address[-10] = LDX_IMM;           // LDX #<abs_address     [2]
                        opcode_address[ -9] = lsb(abs_address);  //
                        opcode_address[ -8] = STX_ZP;            // STX zp_ptr_lo_address [3]
                        opcode_address[ -7] = zp_ptr_lo_address; //
                        opcode_address[ -6] = LDX_IMM;           // LDX #>abs_address     [2]
                        opcode_address[ -5] = msb(abs_address);  //
                        opcode_address[ -4] = STX_ZP;            // STX zp_ptr_hi_address [3]
                        opcode_address[ -3] = zp_ptr_hi_address; //
                        opcode_address[ -2] = LDX_IMM;           // LDX #par3             [2]
                        opcode_address[ -1] = par3;             //
                        opcode_address[  0] = opcode;            // OPC (zpage, X)        [6]
                        opcode_address[  1] = par2;              //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        instruction_cycles = 8;

                        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 10, DEFAULT_RUN_FLAGS, Par1234))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                }
                if (par3 == LAST)
                    break;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_read_modify_write_zpage_indirect_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_ptr_lo_address, address_offset, reg_y

    unsigned zp_ptr_lo_address;
    unsigned zp_ptr_hi_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 2; // This test *DOES* require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            zp_ptr_lo_address = par2;
            zp_ptr_hi_address = (zp_ptr_lo_address + 1) & 0xff;

            if (zp_address_is_safe(zp_ptr_lo_address) && zp_address_is_safe(zp_ptr_hi_address))
            {
                zpage_preserve[0] = zp_ptr_lo_address;
                zpage_preserve[1] = zp_ptr_hi_address;

                for (par3 = 0;;par3 += STEP_SIZE)
                {
                    uint8_t * base_address = TESTCODE_BASE + par3;

                    for (par4 = 0;;par4 += STEP_SIZE)
                    {
                        uint8_t * effective_address = base_address + par4;

                        opcode_address[-10] = LDY_IMM;           // LDY #<base_address    [2]
                        opcode_address[ -9] = lsb(base_address); //
                        opcode_address[ -8] = STY_ZP;            // STY zp_ptr_lo_address [3]
                        opcode_address[ -7] = zp_ptr_lo_address; //
                        opcode_address[ -6] = LDY_IMM;           // LDY #>base_address    [2]
                        opcode_address[ -5] = msb(base_address); //
                        opcode_address[ -4] = STY_ZP;            // STY zp_ptr_hi_address [3]
                        opcode_address[ -3] = zp_ptr_hi_address; //
                        opcode_address[ -2] = LDY_IMM;           // LDY #reg_y            [2]
                        opcode_address[ -1] = par4;              //
                        opcode_address[  0] = opcode;            // OPC (zpage), Y        [5 or 6]
                        opcode_address[  1] = zp_ptr_lo_address; //
                        opcode_address[  2] = OPC_RTS;           // RTS                   [-]

                        test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                        instruction_cycles = 8;

                        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 10, DEFAULT_RUN_FLAGS, Par1234))
                            return false;

                        if (par4 == LAST)
                            break;
                    }
                    if (par3 == LAST)
                        break;
                }
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                            TIMING TESTS FOR BRANCH, JUMP, and INTERRUPT-RELATED INSTRUCTIONS                      //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool timing_test_branch_instruction_taken(const char * test_description, uint8_t opcode, bool flag_value)
{
    // LOOPS: opcode_offset, operand

    // This function tests any of the "branch" instructions, assuming the flag associated with the instruction
    // is in a state that lead to the branch being taken.
    //
    // The flag value (true or false) that leads to the branch being taken is passed in the 'flag_value' parameter.
    // Before executing the branch instructions, the CPU flags that can be used for branching (N, V, Z, C) are all
    // set to this value.
    //
    // Branch instructions, when taken, take 3 or 4 clock cycles:
    //
    // * 3 clock cycles if the address following the branch instruction is on the same memory page as the destination address;
    // * 4 clock cycles if the address following the branch instruction is *not* on the same memory page as the destination address.

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *entry_address = TESTCODE_BASE;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            int displacement = (par2 <= 0x7f) ? par2 : par2 - 0x100;

            if (!(displacement == -1 || displacement == -2))
            {
                // The RTS will not overwrite the branch instruction or its displacement.

                entry_address[0] = OPC_PHP;                        // PHP                  [3]
                entry_address[1] = OPC_PLA;                        // PLA                  [4]
                entry_address[2] = flag_value ? ORA_IMM : AND_IMM; // ORA #$c3 / AND #$3c  [2]
                entry_address[3] = flag_value ?   0xc3  :   0x3c;  //
                entry_address[4] = OPC_PHA;                        // PHA                  [3]
                entry_address[5] = OPC_PLP;                        // PLP                  [4]
                entry_address[6] = JMP_ABS;                        // JMP opcode_address   [3]
                entry_address[7] = lsb(opcode_address);            //
                entry_address[8] = msb(opcode_address);            //

                opcode_address[0] = opcode;                        // Bxx operand          [3 or 4]
                opcode_address[1] = par2;                          //
                opcode_address[2 + displacement] = OPC_RTS;        // RTS                  [-]

                test_overhead_cycles = 3 + 4 + 2 + 3 + 4 + 3;
                instruction_cycles = 3 + different_pages(&opcode_address[2], &opcode_address[2 + displacement]);

                if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, entry_address, DEFAULT_RUN_FLAGS, Par12))
                    return false;
            }
            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

static bool timing_test_branch_instruction_not_taken(const char * test_description, uint8_t opcode, bool flag_value)
{
    // LOOPS: opcode_offset, operand

    // This function tests any of the "branch" instructions, assuming the flag associated with the instruction
    // is in a state that lead to the branch *NOT* being taken.
    //
    // The flag value (true or false) that leads to the branch being not taken is passed in the 'flag_value' parameter.
    // Before executing the branch instructions, the CPU flags that can be used for branching (N, V, Z, C) are all
    // set to this value.
    //
    // Branch instructions, when not taken, always take 2 clock cycles.

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *entry_address  = TESTCODE_BASE;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            entry_address[0] = OPC_PHP;                        // PHP                  [3]
            entry_address[1] = OPC_PLA;                        // PLA                  [4]
            entry_address[2] = flag_value ? ORA_IMM : AND_IMM; // ORA #$C3 / AND #$3C  [2]
            entry_address[3] = flag_value ?   0xc3  :   0x3c;  //
            entry_address[4] = OPC_PHA;                        // PHA                  [3]
            entry_address[5] = OPC_PLP;                        // PLP                  [4]
            entry_address[6] = JMP_ABS;                        // JMP opcode_address   [3]
            entry_address[7] = lsb(opcode_address);            //
            entry_address[8] = msb(opcode_address);            //

            opcode_address[0] = opcode;                        // Bxx operand          [2]
            opcode_address[1] = par2;                          //
            opcode_address[2] = OPC_RTS;                       // RTS                  [-]

            test_overhead_cycles = 3 + 4 + 2 + 3 + 4 + 3;
            instruction_cycles = 2;

            if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, entry_address, DEFAULT_RUN_FLAGS, Par12))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_branch_instruction(const char * test_description, uint8_t opcode, bool flag_value)
{
    // This function tests any of the "branch" instructions, testing both the "branch taken" and "branch not taken"
    // scenarios.
    //
    // The 'flag_value' parameter determines whether "branch taken" happens when the associated flag is High or Low.
    // The instructions BMI, BCS, BVS, and BEQ jump when N/C/V/Z are 1; BPL, BCC, BVC and BNE jump when N/C/V/Z are 0.
    //
    // Branch instructions, when not taken, take 2 clock cycles. When taken, they take 3 or 4 clock cycles:
    //
    // * 3 clock cycles if the address following the branch instruction is on the same memory page as the destination address;
    // * 4 clock cycles if the address following the branch instruction is *not* on the same memory page as the destination address.

    char augmented_test_description[40];

    sprintf(augmented_test_description, "%s - taken", test_description);
    if (!timing_test_branch_instruction_taken(augmented_test_description, opcode, flag_value))
        return false;

    sprintf(augmented_test_description, "%s - not taken", test_description);
    if (!timing_test_branch_instruction_not_taken(augmented_test_description, opcode, !flag_value))
        return false;

    return true;
}

bool timing_test_jmp_abs_instruction(const char * test_description)
{
    // LOOPS: opcode_offset

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0] = JMP_ABS;                 // JMP abs      [3]
        opcode_address[1] = lsb(opcode_address + 3); //
        opcode_address[2] = msb(opcode_address + 3); //
        opcode_address[3] = OPC_RTS;                 // RTS          [-]

        test_overhead_cycles = 0;
        instruction_cycles   = 3;

        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par1))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_jmp_indirect_instruction(const char * test_description)
{
    // LOOPS: opcode_offset, address_offset

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        for (par2 = 0;;par2 += STEP_SIZE)
        {
            uint8_t * target_ptr_address = TESTCODE_BASE + par2;

            target_ptr_address[0] = lsb(opcode_address + 3);
            target_ptr_address[1] = msb(opcode_address + 3);

            // The jump-indirect instruction suffers from the "JUMP INDIRECT" bug on
            // older models of the 6502; this bug was corrected on later models.
            //
            // In case the 6502 we're testing has this bug, we're also putting
            // the MSB of the target at the "wrong" location, in cases where
            // this bug would be triggered.

            if (different_pages(target_ptr_address + 0, target_ptr_address + 1))
                target_ptr_address[1 - 0x100] = msb(opcode_address + 3);

            opcode_address[0] = JMP_IND;                 // JMP ind      [5]
            opcode_address[1] = lsb(target_ptr_address); //
            opcode_address[2] = msb(target_ptr_address); //
            opcode_address[3] = OPC_RTS;                 // RTS          [-]

            test_overhead_cycles = 0;
            instruction_cycles   = 5;

            if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par12))
                return false;

            if (par2 == LAST)
                break;
        }
        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_jsr_abs_instruction(const char * test_description)
{
    // LOOPS: opcode_offset

    unsigned par1;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[0] = JSR_ABS;                 // JSR abs      [6]
        opcode_address[1] = lsb(opcode_address + 3); //
        opcode_address[2] = msb(opcode_address + 3); //
        opcode_address[3] = OPC_PLA;                 // PLA          [4]
        opcode_address[4] = OPC_PLA;                 // PLA          [4]
        opcode_address[5] = OPC_RTS;                 // RTS          [-]

        test_overhead_cycles = 8;
        instruction_cycles   = 6;

        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address, DEFAULT_RUN_FLAGS, Par1))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_rts_instruction(const char * test_description)
{
    // LOOPS: opcode_offset

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        // Note: the RTS is both used as the instruction-under-test, and as the RTS back to
        // the measurement routine.

        opcode_address[-6] = LDA_IMM;                 // LDA #>(rts_address - 1)   [2]
        opcode_address[-5] = msb(opcode_address - 1); //
        opcode_address[-4] = OPC_PHA;                 // PHA                       [3]
        opcode_address[-3] = LDA_IMM;                 // LDA #<(rts_address - 1)   [2]
        opcode_address[-2] = lsb(opcode_address - 1); //
        opcode_address[-1] = OPC_PHA;                 // PHA                       [3]
        opcode_address[ 0] = OPC_RTS;                 // RTS                       [6]

        test_overhead_cycles = 2 + 3 + 2 + 3;
        instruction_cycles   = 6;

        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 6, DEFAULT_RUN_FLAGS, Par1))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_brk_instruction(const char * test_description)
{
    // LOOPS: opcode_offset

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;
    uint8_t *oldvec;
    bool proceed;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[-4] = OPC_TSX;               // TSX          [2]
        opcode_address[-3] = STX_ABS;               // STX save_sp  [4]
        opcode_address[-2] = lsb(opcode_address+6); //
        opcode_address[-1] = msb(opcode_address+6); //
        opcode_address[ 0] = OPC_BRK;               // BRK          [7]
        opcode_address[ 1] = LDX_ABS;               // LDX save_sp  [4] The BRK ends up here.
        opcode_address[ 2] = lsb(opcode_address+6); //
        opcode_address[ 3] = msb(opcode_address+6); //
        opcode_address[ 4] = OPC_TXS;               // TXS          [2]
        opcode_address[ 5] = OPC_RTS;               // RTS          [-]

        test_overhead_cycles = 2 + 4 + TARGET_SPECIFIC_IRQ_OVERHEAD + 4 + 2;
        instruction_cycles   = 7;

        oldvec = set_irq_vector_address(opcode_address + 1);

        proceed = run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 4, DEFAULT_RUN_FLAGS, Par1);

        set_irq_vector_address(oldvec);

        if (!proceed)
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}

bool timing_test_rti_instruction(const char * test_description)
{
    // LOOPS: opcode_offset

    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (par1 = 0;;par1 += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + par1;

        opcode_address[-7] = LDA_IMM;                 // LDA #>rts_address     [2]
        opcode_address[-6] = msb(opcode_address + 1); //
        opcode_address[-5] = OPC_PHA;                 // PHA                   [3]
        opcode_address[-4] = LDA_IMM;                 // LDA #<rts_address     [2]
        opcode_address[-3] = lsb(opcode_address + 1); //
        opcode_address[-2] = OPC_PHA;                 // PHA                   [3]
        opcode_address[-1] = OPC_PHP;                 // PHP                   [3]
        opcode_address[ 0] = OPC_RTI;                 // RTI                   [6]
        opcode_address[ 1] = OPC_RTS;                 // RTS                   [-]

        test_overhead_cycles = 2 + 3 + 2 + 3 + 3;
        instruction_cycles   = 6;

        if (!run_measurement(test_description, test_overhead_cycles, instruction_cycles, opcode_address - 7, DEFAULT_RUN_FLAGS, Par1))
            return false;

        if (par1 == LAST)
            break;
    }
    return true;
}
