
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                   //
//                           TIMING TESTS FOR SINGLE, DOUBLE, AND TRIPLE BYTE INSTRUCTION SEQUENCES                  //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_write_abs_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_x

    unsigned opcode_offset;
    unsigned address_offset, reg_x;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    pre_every_test_hook(test_description);

    num_zpage_preserve = 0; // This test does not require zero page address preservation.

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + address_offset;

            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                opcode_address[-2] = LDX_IMM;           // LDX #imm             [2]
                opcode_address[-1] = reg_x;             //
                opcode_address[ 0] = opcode;            // OPC base_address,X   [5]
                opcode_address[ 1] = lsb(base_address); //
                opcode_address[ 2] = msb(base_address); //
                opcode_address[ 3] = OPC_RTS;           // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 5;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address - 2, DEFAULT_RUN_FLAGS,
                    "opcode offset", opcode_offset,
                    "address offset", address_offset,
                    "X register", reg_x,
                    NULL
                )) return false;
            }
        }
    }
    return true;
}
