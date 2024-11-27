
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
//                           TIMING TESTS FOR SINGLE, DOUBLE, AND TRIPLE BYTE INSTRUCTION SEQUENCES                  //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool timing_test_single_byte_instruction_sequence(const char * test_description, uint8_t b1, unsigned instruction_cycles)
{
    // LOOPS: opcode_offset
    unsigned opcode_offset;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = b1;   // OPC
        opcode_address[1] = 0x60; // RTS [-]

        if (!run_measurement(
            test_description,
            0, instruction_cycles, opcode_address, false,
            "opcode offset", opcode_offset,
            NULL
        )) return false;
    }
    return true;
}

bool timing_test_two_byte_instruction_sequence(const char * test_description, uint8_t b1, uint8_t b2, unsigned test_overhead_cycles, unsigned instruction_cycles)
{
    // LOOPS: opcode_offset
    unsigned opcode_offset;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = b1;   // OPC
        opcode_address[1] = b2;   // OPC
        opcode_address[2] = 0x60; // RTS [-]

        if (!run_measurement(
            test_description,
            test_overhead_cycles, instruction_cycles, opcode_address, false,
            "opcode offset", opcode_offset,
            NULL
        )) return false;
    }
    return true;
}

bool timing_test_three_byte_instruction_sequence(const char * test_description, uint8_t b1, uint8_t b2, uint8_t b3, unsigned test_overhead_cycles, unsigned instruction_cycles)
{
    // LOOPS: opcode_offset
    unsigned opcode_offset;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = b1;   // OPC
        opcode_address[1] = b2;   // OPC
        opcode_address[2] = b3;   // OPC
        opcode_address[3] = 0x60; // RTS [-]

        if (!run_measurement(
            test_description,
            test_overhead_cycles, instruction_cycles, opcode_address, false,
            "opcode offset", opcode_offset,
            NULL
        )) return false;
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
    unsigned opcode_offset;
    unsigned operand;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (operand = 0; operand <= 0xff; operand += STEP_SIZE)
        {
            opcode_address[0] = opcode;     // OPC #imm  [2]
            opcode_address[1] = operand;    //
            opcode_address[2] = 0x60;       // RTS       [-]

            test_overhead_cycles = 0;
            instruction_cycles = 2;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, opcode_address, false,
                "opcode offset", opcode_offset,
                "operand", operand,
                NULL
            )) return false;
        }
    }
    return true;
}

bool timing_test_read_zpage_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address
    unsigned opcode_offset;
    unsigned zp_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            if (!zp_address_is_safe(zp_address))
            {
                continue;
            }

            opcode_address[0] = opcode;     // OPC zp_address  [3]
            opcode_address[1] = zp_address; //
            opcode_address[2] = 0x60;       // RTS             [-]

            test_overhead_cycles = 0;
            instruction_cycles = 3;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, opcode_address, false,
                "opcode offset", opcode_offset,
                "zp address", zp_address,
                NULL
            )) return false;
        }
    }
    return true;
}

bool timing_test_read_zpage_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_x
    unsigned opcode_offset;
    unsigned zp_address, reg_x;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                if (!zp_address_is_safe(zp_address + reg_x))
                {
                    continue;
                }

                opcode_address[0] = 0xa2;       // LDX #imm         [2]
                opcode_address[1] = reg_x;      //
                opcode_address[2] = opcode;     // OPC zp_adress    [4]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS              [-]

                test_overhead_cycles = 2;
                instruction_cycles = 4;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, false,
                    "opcode offset", opcode_offset,
                    "zp address", zp_address,
                    "X register", reg_x,
                    NULL
                )) return false;
            }
        }
    }
    return true;
}

bool timing_test_read_zpage_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_y
    unsigned opcode_offset;
    unsigned zp_address, reg_y;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_y = 0; reg_y <= 0xff; reg_y += STEP_SIZE)
            {
                if (!zp_address_is_safe(zp_address + reg_y))
                {
                    continue;
                }

                opcode_address[0] = 0xa0;       // LDY #imm           [2]
                opcode_address[1] = reg_y;      //
                opcode_address[2] = opcode;     // OPC zp_address     [4]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS                [-]

                test_overhead_cycles = 2;
                instruction_cycles = 4;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, false,
                    "opcode offset", opcode_offset,
                    "zp address", zp_address,
                    "Y register", reg_y,
                    NULL
                )) return false;
            }
        }
    }
    return true;
}

bool timing_test_read_abs_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset
    unsigned opcode_offset;
    unsigned address_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * read_address = TESTCODE_BASE + address_offset;

            opcode_address[0] = opcode;             // OPC read_address   [4]
            opcode_address[1] = lsb(read_address);  //
            opcode_address[2] = msb(read_address);  //
            opcode_address[3] = 0x60;               // RTS                [-]

            test_overhead_cycles = 0;
            instruction_cycles = 4;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, opcode_address, false,
                "opcode offset", opcode_offset,
                "address offset", address_offset,
                NULL
            )) return false;
        }
    }
    return true;
}

bool timing_test_read_abs_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_x
    unsigned opcode_offset;
    unsigned address_offset, reg_x;
    unsigned instruction_cycles, test_overhead_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + address_offset;

            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                opcode_address[0] = 0xa2;               // LDX #imm             [2]
                opcode_address[1] = reg_x;              //
                opcode_address[2] = opcode;             // OPC base_address,X   [4 or 5]
                opcode_address[3] = lsb(base_address);  //
                opcode_address[4] = msb(base_address);  //
                opcode_address[5] = 0x60;               // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 4 + different_pages(base_address, base_address + reg_x);

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, false,
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

bool timing_test_read_abs_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_y
    unsigned opcode_offset;
    unsigned address_offset, reg_y;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + address_offset;

            for (reg_y = 0; reg_y <= 0xff; reg_y += STEP_SIZE)
            {
                opcode_address[0] = 0xa0;               // LDY #imm             [2]
                opcode_address[1] = reg_y;              //
                opcode_address[2] = opcode;             // OPC base_address,Y   [4 or 5]
                opcode_address[3] = lsb(base_address);  //
                opcode_address[4] = msb(base_address);  //
                opcode_address[5] = 0x60;               // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 4 + different_pages(base_address, base_address + reg_y);

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, false,
                    "opcode offset", opcode_offset,
                    "address offset", address_offset,
                    "Y register", reg_y,
                    NULL
                )) return false;
            }
        }
    }
    return true;
}

bool timing_test_read_zpage_x_indirect_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_base_address, reg_x, address_offset
    unsigned opcode_offset;
    unsigned zp_base_address;
    unsigned reg_x;
    unsigned address_offset;
    unsigned zp_ptr_lo_address;
    unsigned zp_ptr_hi_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_base_address = 0; zp_base_address <= 0xff; zp_base_address += STEP_SIZE)
        {
            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                // (zp_ptr_lo, zp_ptr_hi) will be the actual pointer used for indirection.
                zp_ptr_lo_address = (zp_base_address + reg_x + 0) & 0xff;
                zp_ptr_hi_address = (zp_base_address + reg_x + 1) & 0xff;

                if (!zp_address_is_safe(zp_ptr_lo_address) || !zp_address_is_safe(zp_ptr_hi_address))
                {
                    continue;
                }

                for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
                {
                    uint8_t * abs_address = TESTCODE_BASE + address_offset;

                    opcode_address[ 0] = 0xa2;              // LDX #<abs_address     [2]
                    opcode_address[ 1] = lsb(abs_address);  //
                    opcode_address[ 2] = 0x86;              // STX zp_ptr_lo_address [3]
                    opcode_address[ 3] = zp_ptr_lo_address; //
                    opcode_address[ 4] = 0xa2;              // LDX #>abs_address     [2]
                    opcode_address[ 5] = msb(abs_address);  //
                    opcode_address[ 6] = 0x86;              // STX zp_ptr_hi_address [3]
                    opcode_address[ 7] = zp_ptr_hi_address; //
                    opcode_address[ 8] = 0xa2;              // LDX #reg_x            [2]
                    opcode_address[ 9] = reg_x;             //
                    opcode_address[10] = opcode;            // OPC (zpage, X)        [6]
                    opcode_address[11] = zp_base_address;   //
                    opcode_address[12] = 0x60;              // RTS                   [-]

                    test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                    instruction_cycles = 6;

                    if (!run_measurement(
                        test_description,
                        test_overhead_cycles, instruction_cycles, opcode_address, true,
                        "opcode offset", opcode_offset,
                        "zp base address", zp_base_address,
                        "X register", reg_x,
                        "address offset", address_offset,
                        NULL
                    )) return false;
                }
            }
        }
    }
    return true;
}

bool timing_test_read_zpage_indirect_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_ptr_lo_address, address_offset, reg_y
    unsigned opcode_offset;
    unsigned address_offset;
    unsigned reg_y;
    unsigned zp_ptr_lo_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_ptr_lo_address = 0; zp_ptr_lo_address <= 0xff; zp_ptr_lo_address += STEP_SIZE)
        {
            unsigned zp_ptr_hi_address = (zp_ptr_lo_address + 1) & 0xff;

            if (!zp_address_is_safe(zp_ptr_lo_address) || !zp_address_is_safe(zp_ptr_hi_address))
            {
                continue;
            }

            for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
            {
                uint8_t * base_address = TESTCODE_BASE + address_offset;

                for (reg_y = 0; reg_y <= 0xff; reg_y += STEP_SIZE)
                {
                    uint8_t * effective_address = base_address + reg_y;

                    opcode_address[ 0] = 0xa0;              // LDY #<base_address    [2]
                    opcode_address[ 1] = lsb(base_address); //
                    opcode_address[ 2] = 0x84;              // STY zp_ptr_lo_address [3]
                    opcode_address[ 3] = zp_ptr_lo_address; //
                    opcode_address[ 4] = 0xa0;              // LDY #>base_address    [2]
                    opcode_address[ 5] = msb(base_address); //
                    opcode_address[ 6] = 0x84;              // STY zp_ptr_hi_address [3]
                    opcode_address[ 7] = zp_ptr_hi_address; //
                    opcode_address[ 8] = 0xa0;              // LDY #reg_y            [2]
                    opcode_address[ 9] = reg_y;             //
                    opcode_address[10] = opcode;            // OPC (zpage), Y        [5 or 6]
                    opcode_address[11] = zp_ptr_lo_address; //
                    opcode_address[12] = 0x60;              // RTS                   [-]

                    test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                    instruction_cycles = 5 + different_pages(base_address, effective_address);

                    if (!run_measurement(
                        test_description,
                        test_overhead_cycles, instruction_cycles, opcode_address, true,
                        "opcode offset", opcode_offset,
                        "zp ptr address", zp_ptr_lo_address,
                        "address offset", address_offset,
                        "Y register", reg_y,
                        NULL
                    )) return false;
                }
            }
        }
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
    unsigned opcode_offset;
    unsigned zp_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            if (!zp_address_is_safe(zp_address))
            {
                continue;
            }

            opcode_address[0] = opcode;     // OPC zp_address   [3]
            opcode_address[1] = zp_address; //
            opcode_address[2] = 0x60;       // RTS              [-]

            test_overhead_cycles = 0;
            instruction_cycles = 3;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, opcode_address, true,
                "opcode offset", opcode_offset,
                "zp address", zp_address,
                NULL
            )) return false;
        }
    }
    return true;
}

bool timing_test_write_zpage_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_x
    unsigned opcode_offset;
    unsigned zp_address, reg_x;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                if (!zp_address_is_safe(zp_address + reg_x))
                {
                    continue;
                }

                opcode_address[0] = 0xa2;       // LDX #imm             [2]
                opcode_address[1] = reg_x;      //
                opcode_address[2] = opcode;     // OPC zp_address,X     [4]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS                  [1]

                test_overhead_cycles = 2;
                instruction_cycles = 4;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, true,
                    "opcode offset", opcode_offset,
                    "zp address", zp_address,
                    "X register", reg_x,
                    NULL
                )) return false;
            }
        }
    }
    return true;
}

bool timing_test_write_zpage_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_y
    unsigned opcode_offset;
    unsigned zp_address, reg_y;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_y = 0; reg_y <= 0xff; reg_y += STEP_SIZE)
            {
                if (!zp_address_is_safe(zp_address + reg_y))
                {
                    continue;
                }

                opcode_address[0] = 0xa0;       // LDY #imm          [2]
                opcode_address[1] = reg_y;      //
                opcode_address[2] = opcode;     // OPC zp_address,Y  [4]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS               [-]

                test_overhead_cycles = 2;
                instruction_cycles = 4;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, true,
                    "opcode offset", opcode_offset,
                    "zp address", zp_address,
                    "Y register", reg_y,
                    NULL
                )) return false;
            }
        }
    }
    return true;
}

bool timing_test_write_abs_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset
    unsigned opcode_offset;
    unsigned address_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * write_address = TESTCODE_BASE + address_offset;

            opcode_address[0] = opcode;             // OPC write_address   [4]
            opcode_address[1] = lsb(write_address); //
            opcode_address[2] = msb(write_address); //
            opcode_address[3] = 0x60;               // RTS                 [-]

            test_overhead_cycles = 0;
            instruction_cycles = 4;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, opcode_address, false,
                "opcode offset", opcode_offset,
                "address offset", address_offset,
                NULL
            )) return false;
        }
    }
    return true;
}

bool timing_test_write_abs_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_x
    unsigned opcode_offset;
    unsigned address_offset, reg_x;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + address_offset;

            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                opcode_address[0] = 0xa2;               // LDX #imm             [2]
                opcode_address[1] = reg_x;              //
                opcode_address[2] = opcode;             // OPC base_address,X   [5]
                opcode_address[3] = lsb(base_address);  //
                opcode_address[4] = msb(base_address);  //
                opcode_address[5] = 0x60;               // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 5;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, false,
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

bool timing_test_write_abs_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_y
    unsigned opcode_offset;
    unsigned address_offset, reg_y;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + address_offset;

            for (reg_y = 0; reg_y <= 0xff; reg_y += STEP_SIZE)
            {
                opcode_address[0] = 0xa0;               // LDY #imm             [2]
                opcode_address[1] = reg_y;              //
                opcode_address[2] = opcode;             // OPC base_address,Y   [5]
                opcode_address[3] = lsb(base_address);  //
                opcode_address[4] = msb(base_address);  //
                opcode_address[5] = 0x60;               // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 5;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, false,
                    "opcode offset", opcode_offset,
                    "address offset", address_offset,
                    "Y register", reg_y,
                    NULL
                )) return false;
            }
        }
    }
    return true;
}

bool timing_test_write_zpage_x_indirect_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_base_address, reg_x, address_offset
    unsigned opcode_offset;
    unsigned zp_base_address;
    unsigned reg_x;
    unsigned address_offset;
    unsigned zp_ptr_lo_address;
    unsigned zp_ptr_hi_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_base_address = 0; zp_base_address <= 0xff; zp_base_address += STEP_SIZE)
        {
            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                // (zp_ptr_lo, zp_ptr_hi) will be the actual pointer used for indirection.
                zp_ptr_lo_address = (zp_base_address + reg_x + 0) & 0xff;
                zp_ptr_hi_address = (zp_base_address + reg_x + 1) & 0xff;

                if (!zp_address_is_safe(zp_ptr_lo_address) || !zp_address_is_safe(zp_ptr_hi_address))
                {
                    continue;
                }

                for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
                {
                    uint8_t * abs_address = TESTCODE_BASE + address_offset;

                    opcode_address[ 0] = 0xa2;              // LDX #<abs_address     [2]
                    opcode_address[ 1] = lsb(abs_address);  //
                    opcode_address[ 2] = 0x86;              // STX zp_ptr_lo_address [3]
                    opcode_address[ 3] = zp_ptr_lo_address; //
                    opcode_address[ 4] = 0xa2;              // LDX #>abs_address     [2]
                    opcode_address[ 5] = msb(abs_address);  //
                    opcode_address[ 6] = 0x86;              // STX zp_ptr_hi_address [3]
                    opcode_address[ 7] = zp_ptr_hi_address; //
                    opcode_address[ 8] = 0xa2;              // LDX #reg_x            [2]
                    opcode_address[ 9] = reg_x;             //
                    opcode_address[10] = opcode;            // OPC (zpage, X)        [6]
                    opcode_address[11] = zp_base_address;   //
                    opcode_address[12] = 0x60;              // RTS                   [-]

                    test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                    instruction_cycles = 6;

                    if (!run_measurement(
                        test_description,
                        test_overhead_cycles, instruction_cycles, opcode_address, true,
                        "opcode offset", opcode_offset,
                        "zp base address", zp_base_address,
                        "address offset", address_offset,
                        "X register", reg_x,
                        NULL
                    )) return false;
                }
            }
        }
    }
    return true;
}

bool timing_test_write_zpage_indirect_y_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_ptr_lo_address, address_offset, reg_y
    unsigned opcode_offset;
    unsigned address_offset;
    unsigned reg_y;
    unsigned zp_ptr_lo_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_ptr_lo_address = 0; zp_ptr_lo_address <= 0xff; zp_ptr_lo_address += STEP_SIZE)
        {
            unsigned zp_ptr_hi_address = (zp_ptr_lo_address + 1) & 0xff;

            if (!zp_address_is_safe(zp_ptr_lo_address) || !zp_address_is_safe(zp_ptr_hi_address))
            {
                continue;
            }

            for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
            {
                uint8_t * base_address = TESTCODE_BASE + address_offset;

                for (reg_y = 0; reg_y <= 0xff; reg_y += STEP_SIZE)
                {
                    uint8_t * effective_address = base_address + reg_y;

                    opcode_address[ 0] = 0xa0;              // LDY #<base_address    [2]
                    opcode_address[ 1] = lsb(base_address); //
                    opcode_address[ 2] = 0x84;              // STY zp_ptr_lo_address [3]
                    opcode_address[ 3] = zp_ptr_lo_address; //
                    opcode_address[ 4] = 0xa0;              // LDY #>base_address    [2]
                    opcode_address[ 5] = msb(base_address); //
                    opcode_address[ 6] = 0x84;              // STY zp_ptr_hi_address [3]
                    opcode_address[ 7] = zp_ptr_hi_address; //
                    opcode_address[ 8] = 0xa0;              // LDY #reg_y            [2]
                    opcode_address[ 9] = reg_y;             //
                    opcode_address[10] = opcode;            // OPC (zpage), Y        [5 or 6]
                    opcode_address[11] = zp_ptr_lo_address; //
                    opcode_address[12] = 0x60;              // RTS                   [-]

                    test_overhead_cycles = 2 + 3 + 2 + 3 + 2;
                    instruction_cycles = 6;

                if (!run_measurement(
                        test_description,
                        test_overhead_cycles, instruction_cycles, opcode_address, true,
                        "opcode offset", opcode_offset,
                        "zp address", zp_ptr_lo_address,
                        "address offset", address_offset,
                        "Y register", reg_y,
                        NULL
                    )) return false;
                }
            }
        }
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
    unsigned opcode_offset;
    unsigned zp_address;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            if (!zp_address_is_safe(zp_address))
            {
                continue;
            }

            opcode_address[0] = opcode;     // OPC zp_address      [5]
            opcode_address[1] = zp_address; //
            opcode_address[2] = 0x60;       // RTS                 [-]

            test_overhead_cycles = 0;
            instruction_cycles = 5;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, opcode_address, true,
                "opcode offset", opcode_offset,
                "zp address", zp_address,
                NULL
            )) return false;
        }
    }
    return true;
}

bool timing_test_read_modify_write_zpage_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, zp_address, reg_x
    unsigned opcode_offset;
    unsigned zp_address, reg_x;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                if (!zp_address_is_safe(zp_address + reg_x))
                {
                    continue;
                }

                opcode_address[0] = 0xa2;       // LDX #imm            [2]
                opcode_address[1] = reg_x;      //
                opcode_address[2] = opcode;     // OPC zp_address,X    [6]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS                 [-]

                test_overhead_cycles = 2;
                instruction_cycles = 6;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, true,
                    "opcode offset", opcode_offset,
                    "zp address", zp_address,
                    "X register", reg_x,
                    NULL
                )) return false;
            }
        }
    }
    return true;
}

bool timing_test_read_modify_write_abs_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset
    unsigned opcode_offset;
    unsigned address_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * abs_address = TESTCODE_BASE + address_offset;

            opcode_address[0] = opcode;           // OPC abs_address    [6]
            opcode_address[1] = lsb(abs_address); //
            opcode_address[2] = msb(abs_address); //
            opcode_address[3] = 0x60;             // RTS                [-]

            test_overhead_cycles = 0;
            instruction_cycles = 6;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, opcode_address, false,
                "opcode offset", opcode_offset,
                "address offset", address_offset,
                NULL
            )) return false;
        }
    }
    return true;
}

bool timing_test_read_modify_write_abs_x_instruction(const char * test_description, uint8_t opcode)
{
    // LOOPS: opcode_offset, address_offset, reg_x
    unsigned opcode_offset;
    unsigned address_offset, reg_x;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * base_address = TESTCODE_BASE + address_offset;

            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                opcode_address[0] = 0xa2;               // LDX #imm             [2]
                opcode_address[1] = reg_x;              //
                opcode_address[2] = opcode;             // OPC base_address,X   [7]
                opcode_address[3] = lsb(base_address);  //
                opcode_address[4] = msb(base_address);  //
                opcode_address[5] = 0x60;               // RTS                  [-]

                test_overhead_cycles = 2;
                instruction_cycles = 7;

                if (!run_measurement(
                    test_description,
                    test_overhead_cycles, instruction_cycles, opcode_address, false,
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

    unsigned opcode_offset;
    unsigned operand;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *entry_address;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        entry_address = TESTCODE_BASE;
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (operand = 0; operand <= 0xff; operand += STEP_SIZE)
        {
            int displacement = (operand <= 0x7f) ? operand : operand - 0x100;

            if (displacement == -1 || displacement == -2)
            {
                // Putting an RTS here would overwrite the branch instruction itself; skip.
                continue;
            }

            entry_address[0] = 0x08;                        // PHP                  [3]
            entry_address[1] = 0x68;                        // PLA                  [4]
            entry_address[2] = flag_value ? 0x09 : 0x29;    // ORA #$C3 / AND #$3C  [2]
            entry_address[3] = flag_value ? 0xc3 : 0x3c;    //
            entry_address[4] = 0x48;                        // PHA                  [3]
            entry_address[5] = 0x28;                        // PLP                  [4]
            entry_address[6] = 0x4c;                        // JMP opcode_address   [3]
            entry_address[7] = lsb(opcode_address);         //
            entry_address[8] = msb(opcode_address);         //

            opcode_address[0] = opcode;                     // Bxx operand          [3 or 4]
            opcode_address[1] = operand;                    //
            opcode_address[2 + displacement] = 0x60;        // RTS                  [-]

            test_overhead_cycles = 3 + 4 + 2 + 3 + 4 + 3;
            instruction_cycles = 3 + different_pages(&opcode_address[2], &opcode_address[2 + displacement]);

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, entry_address, false,
                "opcode offset", opcode_offset,
                "operand", operand,
                NULL
            )) return false;
        }
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

    unsigned opcode_offset;
    unsigned operand;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *entry_address;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        entry_address = TESTCODE_BASE;
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (operand = 0; operand <= 0xff; operand += STEP_SIZE)
        {
            entry_address[0] = 0x08;                        // PHP                  [3]
            entry_address[1] = 0x68;                        // PLA                  [4]
            entry_address[2] = flag_value ? 0x09 : 0x29;    // ORA #$C3 / AND #$3C  [2]
            entry_address[3] = flag_value ? 0xc3 : 0x3c;    // 
            entry_address[4] = 0x48;                        // PHA                  [3]
            entry_address[5] = 0x28;                        // PLP                  [4]
            entry_address[6] = 0x4c;                        // JMP opcode_address   [3]
            entry_address[7] = lsb(opcode_address);         //
            entry_address[8] = msb(opcode_address);         //

            opcode_address[0] = opcode;                     // Bxx operand          [2]
            opcode_address[1] = operand;                    //
            opcode_address[2] = 0x60;                       // RTS                  [-]

            test_overhead_cycles = 3 + 4 + 2 + 3 + 4 + 3;
            instruction_cycles = 2;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, entry_address, false,
                "opcode offset", opcode_offset,
                "operand", operand,
                NULL
            )) return false;
        }
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
    {
        return false;
    }

    sprintf(augmented_test_description, "%s - not taken", test_description);
    if (!timing_test_branch_instruction_not_taken(augmented_test_description, opcode, !flag_value))
    {
        return false;
    }

    return true;
}

bool timing_test_jmp_abs_instruction(const char * test_description)
{
    // LOOPS: opcode_offset
    unsigned opcode_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = 0x4c;                      // JMP abs      [3]
        opcode_address[1] = lsb(opcode_address + 3);   //
        opcode_address[2] = msb(opcode_address + 3);   //
        opcode_address[3] = 0x60;                      // RTS          [-]

        test_overhead_cycles = 0;
        instruction_cycles   = 3;

        if (!run_measurement(
            test_description,
            test_overhead_cycles, instruction_cycles, opcode_address, false,
            "opcode offset", opcode_offset,
            NULL
        )) return false;
    }
    return true;
}

bool timing_test_jmp_indirect_instruction(const char * test_description)
{
    // LOOPS: opcode_offset, address_offset
    unsigned opcode_offset, address_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * target_ptr_address = TESTCODE_BASE + address_offset;

            target_ptr_address[0] = lsb(opcode_address + 3);
            target_ptr_address[1] = msb(opcode_address + 3);

            // The jump-indirect instruction suffers from the "JUMP INDIRECT" bug on
            // older models of the 6502; this bug was corrected on later models.
            //
            // In case the 6502 we're testing has this bug, we're also putting
            // the MSB of the target at the "wrong" location, in cases where
            // this bug would be triggered.

            if (different_pages(target_ptr_address + 0, target_ptr_address + 1))
            {
                target_ptr_address[1 - 0x100] = msb(opcode_address + 3);
            }

            opcode_address[0] = 0x6c;                      // JMP ind      [5]
            opcode_address[1] = lsb(target_ptr_address);   //
            opcode_address[2] = msb(target_ptr_address);   //
            opcode_address[3] = 0x60;                      // RTS          [-]

            test_overhead_cycles = 0;
            instruction_cycles   = 5;

            if (!run_measurement(
                test_description,
                test_overhead_cycles, instruction_cycles, opcode_address, false,
                "opcode offset", opcode_offset,
                NULL
            )) return false;
        }
    }
    return true;
}

bool timing_test_jsr_abs_instruction(const char * test_description)
{
    // LOOPS: opcode_offset
    unsigned opcode_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = 0x20;                      // JSR abs      [6]
        opcode_address[1] = lsb(opcode_address + 3);   //
        opcode_address[2] = msb(opcode_address + 3);   //
        opcode_address[3] = 0x68;                      // PLA          [4]
        opcode_address[4] = 0x68;                      // PLA          [4]
        opcode_address[5] = 0x60;                      // RTS          [-]

        test_overhead_cycles = 8;
        instruction_cycles   = 6;

        if (!run_measurement(
            test_description,
            test_overhead_cycles, instruction_cycles, opcode_address, false,
            "opcode offset", opcode_offset,
            NULL
        )) return false;
    }
    return true;
}


bool timing_test_rts_instruction(const char * test_description)
{
    // LOOPS: opcode_offset
    unsigned opcode_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        // Note: the RTS is both used as the instruction-under-test, and as the RTS back to
        // the measurement routine.

        opcode_address[0] = 0xa9;                      // LDA #>(rts_address - 1)   [2]
        opcode_address[1] = msb(opcode_address + 5);   //
        opcode_address[2] = 0x48;                      // PHA                       [3]
        opcode_address[3] = 0xa9;                      // LDA #<(rts_address - 1)   [2]
        opcode_address[4] = lsb(opcode_address + 5);   //
        opcode_address[5] = 0x48;                      // PHA                       [3]
        opcode_address[6] = 0x60;                      // RTS                       [6]

        test_overhead_cycles = 2 + 3 + 2 + 3;
        instruction_cycles   = 6;

        if (!run_measurement(
            test_description,
            test_overhead_cycles, instruction_cycles, opcode_address, false,
            "opcode offset", opcode_offset,
            NULL
        )) return false;
    }
    return true;
}

bool timing_test_brk_instruction(const char * test_description)
{
    // LOOPS: opcode_offset
    unsigned opcode_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;
    uint8_t *oldvec;
    bool success;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = 0xba;                      // TSX          [2]
        opcode_address[1] = 0x8e;                      // STX save_sp  [4]
        opcode_address[2] = lsb(TESTCODE_BASE);        //
        opcode_address[3] = msb(TESTCODE_BASE);        //
        opcode_address[4] = 0x00;                      // BRK          [7]
        opcode_address[5] = 0xae;                      // LDX save_sp  [4]
        opcode_address[6] = lsb(TESTCODE_BASE);        //
        opcode_address[7] = msb(TESTCODE_BASE);        //
        opcode_address[8] = 0x9a;                      // TXS          [2]
        opcode_address[9] = 0x60;                      // RTS          [-]

        test_overhead_cycles = 2 + 4 + PLATFORM_SPECIFIC_IRQ_OVERHEAD + 4 + 2;
        instruction_cycles   = 7;

        oldvec = set_irq_vector_address(opcode_address + 5);

        success = run_measurement(
            test_description,
            test_overhead_cycles, instruction_cycles, opcode_address, false,
            "opcode offset", opcode_offset,
            NULL
        );

        set_irq_vector_address(oldvec);

        if (!success)
        {
            return false;
        }
    }
    return true;
}

bool timing_test_rti_instruction(const char * test_description)
{
    // LOOPS: opcode_offset
    unsigned opcode_offset;
    unsigned test_overhead_cycles, instruction_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = 0xa9;                      // LDA #>rts_address     [2]
        opcode_address[1] = msb(opcode_address + 8);   //
        opcode_address[2] = 0x48;                      // PHA                   [3]
        opcode_address[3] = 0xa9;                      // LDA #<rts_address     [2]
        opcode_address[4] = lsb(opcode_address + 8);   //
        opcode_address[5] = 0x48;                      // PHA                   [3]
        opcode_address[6] = 0x08;                      // PHP                   [3]
        opcode_address[7] = 0x40;                      // RTI                   [6]
        opcode_address[8] = 0x60;                      // RTS                   [-]

        test_overhead_cycles = 2 + 3 + 2 + 3 + 3;
        instruction_cycles   = 6;

        if (!run_measurement(
            test_description,
            test_overhead_cycles, instruction_cycles, opcode_address, false,
            "opcode offset", opcode_offset,
            NULL
        ))
        {
            return false;
        }
    }
    return true;
}
