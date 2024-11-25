
///////////////////////////////
// test_instruction_clocks.c //
///////////////////////////////

// This program tests the instruction timing (i.e., clock-cycle counts) for all
// (or at least most) documented 151 6502 instructions.
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
// which is usually in use by the external environment. This function depends on the
// hardware-platform dependent measure_cycles(), but is itself hardware-platform independent:
//
// * measure_cycles_zp_safe()   Save the contents of the zero page, execute measure_cycles(),
//                              then restore the content of the zero page.
//
// ASSUMPTIONS
// -----------
//
// * The program assumes the instructions work correctly, other than that their clock cycle
//   counts may be off.
//
// * Later tests assume that the clock cycle counts of 6502 instructions that were timed
//   by earlier (simpler) instructions are correct.

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "measure_cycles.h"

// For testing purposes we want to be able to go through a bunch of values in the range 0..255 with small or big steps.
// 
// For example, by taking steps of 17, we only examine {0, 17, 34, ..., 238, 255} -- just 16 out of the 256 values.
//
// It is recommended to pick this value as a divisor of 255, so one of 1, 3, 5, 15, 17, 51, 85, and 255.
//
// For a full torture test, use STEP_SIZE==1.
// For a quick-and-dirty run-through (that will already expose quite a few timing mismatches if they are there),
//   use STEP_SIZE=85 or even STEP_SIZE=255.

unsigned STEP_SIZE = 85;

uint8_t * TESTCODE_PTR    = NULL; // The pointer to the full test area, allocated using malloc().
uint8_t * TESTCODE_BASE   = NULL; // The first address in the TESTCODE range that is on a page boundary.
uint8_t * TESTCODE_ANCHOR = NULL; // The halfway point in the TESTCODE range, also on a page boundary. Put test code here.

unsigned  TESTCODE_SIZE   = 0;

unsigned long test_count;
unsigned long error_count;

int allocate_testcode_block(unsigned size)
{
    unsigned offset;

    if (size % 512 != 0)
    {
        // We are only willing to allocate an even number of pages; report failure.
        return -1;
    }

    // We want a paged-aligned block of size 'size'.
    // To guarantee that we get that, allocate 255 bytes more.

    TESTCODE_PTR = malloc(size + 255);
    if (TESTCODE_PTR == NULL)
    {
        // Unable to allocate the required memory; report failure.
        return -1;
    }

    // Where does the first page start?

    offset = (256 - (unsigned)TESTCODE_PTR % 256) % 256;

    // Initialize the important variables.

    TESTCODE_BASE = TESTCODE_PTR + offset;
    TESTCODE_ANCHOR = TESTCODE_BASE + size / 2;
    TESTCODE_SIZE = size;

    return 0;
}

void free_testcode_block(void)
{
    free(TESTCODE_PTR);
}

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
    unsigned wanted_cycles, actual_cycles, rep;

    for (rep = 1; rep <= repeats; ++rep)
    {
        for (wanted_cycles = min_cycle_count; wanted_cycles <= max_cycle_count; ++wanted_cycles)
        {
            if (wanted_cycles == 1)
            {
                // Cannot generate 1-cycle test code.
                continue;
            }

            generate_code(TESTCODE_BASE, wanted_cycles);

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(TESTCODE_BASE);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != wanted_cycles)
            {
                printf("[%u] ERROR: wanted cycles: %u -> actual cycles %d\n", rep, wanted_cycles, actual_cycles);
                ++error_count;
            }
        }
    }
}

uint8_t lsb(uint8_t * ptr)
{
    return (uint16_t)ptr & 0xff;
}

uint8_t msb(uint8_t * ptr)
{
    return (uint16_t)ptr >> 8;
}

bool different_pages(uint8_t * u1, uint8_t * u2)
{
    return msb(u1) != msb(u2);
}

void test_branch_taken(char * test_description, uint8_t branch_opcode, bool flag_value)
{
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
    unsigned predicted_cycles, actual_cycles;
    uint8_t *entry_address;
    uint8_t *branch_opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u operands ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        entry_address = TESTCODE_BASE;
        branch_opcode_address = TESTCODE_ANCHOR + opcode_offset;

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
            entry_address[6] = 0x4c;                        // JMP branch_opcode    [3]
            entry_address[7] = lsb(branch_opcode_address);  //
            entry_address[8] = msb(branch_opcode_address);  //

            branch_opcode_address[0] = branch_opcode;       // Bxx operand          [3 or 4]
            branch_opcode_address[1] = operand;             //
            branch_opcode_address[2 + displacement] = 0x60; // RTS                  [-]

            predicted_cycles = 3 + 4 + 2 + 3 + 4 + 3 + 3 + different_pages(&branch_opcode_address[2], &branch_opcode_address[2 + displacement]);

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(entry_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x operand %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, operand,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}

void test_branch_not_taken(char * test_description, uint8_t branch_opcode, bool flag_value)
{
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
    unsigned predicted_cycles, actual_cycles;
    uint8_t *entry_address;
    uint8_t *branch_opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u operands ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        entry_address = TESTCODE_BASE;
        branch_opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (operand = 0; operand <= 0xff; operand += STEP_SIZE)
        {
            entry_address[0] = 0x08;                        // PHP                  [3]
            entry_address[1] = 0x68;                        // PLA                  [4]
            entry_address[2] = flag_value ? 0x09 : 0x29;    // ORA #$C3 / AND #$3C  [2]
            entry_address[3] = flag_value ? 0xc3 : 0x3c;    // 
            entry_address[4] = 0x48;                        // PHA                  [3]
            entry_address[5] = 0x28;                        // PLP                  [4]
            entry_address[6] = 0x4c;                        // JMP branch_opcode    [3]
            entry_address[7] = lsb(branch_opcode_address);  //
            entry_address[8] = msb(branch_opcode_address);  //

            branch_opcode_address[0] = branch_opcode;       // Bxx operand          [2]
            branch_opcode_address[1] = operand;             //
            branch_opcode_address[2] = 0x60;                // RTS                  [-]

            predicted_cycles = 3 + 4 + 2 + 3 + 4 + 3 + 2;

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(entry_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x operand %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, operand,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}

void test_single_byte_instruction_sequence(char * test_description, uint8_t b1, unsigned predicted_cycles)
{
    unsigned opcode_offset;
    unsigned actual_cycles;
    uint8_t *opcode_address;

    printf("[%lu] INFO (%s) testing %u opcode offsets ...\n",
            error_count, test_description, 1 + 255 / STEP_SIZE);

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = b1;   // OPC
        opcode_address[1] = 0x60; // RTS [-]

        dma_and_interrupts_off();
        actual_cycles = measure_cycles(opcode_address);
        dma_and_interrupts_on();
        ++test_count;

        if (actual_cycles != predicted_cycles)
        {
            ++error_count;
            printf("[%lu] ERROR (%s) opcode offset %02x cycles p/a %u/%u\n",
                error_count,
                test_description,
                opcode_offset,
                predicted_cycles, actual_cycles);
        }
    }
}

void test_two_byte_instruction_sequence(char * test_description, uint8_t b1, uint8_t b2, unsigned predicted_cycles)
{
    unsigned opcode_offset;
    unsigned actual_cycles;
    uint8_t *opcode_address;

    printf("[%lu] INFO (%s) testing %u opcode offsets ...\n",
            error_count, test_description, 1 + 255 / STEP_SIZE);

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = b1;   // OPC
        opcode_address[1] = b2;   // OPC
        opcode_address[2] = 0x60; // RTS [-]

        dma_and_interrupts_off();
        actual_cycles = measure_cycles(opcode_address);
        dma_and_interrupts_on();
        ++test_count;

        if (actual_cycles != predicted_cycles)
        {
            ++error_count;
            printf("[%lu] ERROR (%s) opcode offset %02x cycles p/a %u/%u\n",
                error_count,
                test_description,
                opcode_offset,
                predicted_cycles, actual_cycles);
        }
    }
}

void test_three_byte_instruction_sequence(char * test_description, uint8_t b1, uint8_t b2, uint8_t b3, unsigned predicted_cycles)
{
    unsigned opcode_offset;
    unsigned actual_cycles;
    uint8_t *opcode_address;

    printf("[%lu] INFO (%s) testing %u opcode offsets ...\n",
            error_count, test_description, 1 + 255 / STEP_SIZE);

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        opcode_address[0] = b1;   // OPC
        opcode_address[1] = b2;   // OPC
        opcode_address[2] = b3;   // OPC
        opcode_address[3] = 0x60; // RTS [-]

        dma_and_interrupts_off();
        actual_cycles = measure_cycles(opcode_address);
        dma_and_interrupts_on();
        ++test_count;

        if (actual_cycles != predicted_cycles)
        {
            ++error_count;
            printf("[%lu] ERROR (%s) opcode offset %02x cycles p/a %u/%u\n",
                error_count,
                test_description,
                opcode_offset,
                predicted_cycles, actual_cycles);
        }
    }
}

void test_immediate_mode_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned operand;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u operands ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (operand = 0; operand <= 0xff; operand += STEP_SIZE)
        {
            opcode_address[0] = opcode;     // OPC #imm  [2]
            opcode_address[1] = operand;    //
            opcode_address[2] = 0x60;       // RTS       [-]

            predicted_cycles = 2;

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(opcode_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x operand %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, operand,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}

void test_read_zpage_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned zp_address;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u zp addresses ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            opcode_address[0] = opcode;     // OPC zp_address  [3]
            opcode_address[1] = zp_address; //
            opcode_address[2] = 0x60;       // RTS             [-]

            predicted_cycles = 3;

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(opcode_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x zp address %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, zp_address,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}

void test_read_zpage_x_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned zp_address, reg_x;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u zp addresses ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                opcode_address[0] = 0xa2;       // LDX #imm         [2]
                opcode_address[1] = reg_x;      //
                opcode_address[2] = opcode;     // OPC zp_adress    [4]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS              [-]

                predicted_cycles = 2 + 4;

                dma_and_interrupts_off();
                actual_cycles = measure_cycles(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x zp address %02x X %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, zp_address, reg_x,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_read_zpage_y_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned zp_address, reg_y;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u zp addresses ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_y = 0; reg_y <= 0xff; reg_y += STEP_SIZE)
            {
                opcode_address[0] = 0xa0;       // LDY #imm           [2]
                opcode_address[1] = reg_y;      //
                opcode_address[2] = opcode;     // OPC zp_address     [4]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS                [-]

                predicted_cycles = 2 + 4;

                dma_and_interrupts_off();
                actual_cycles = measure_cycles(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x zp address %02x Y %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, zp_address, reg_y,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_read_abs_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned address_offset;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * read_address = TESTCODE_BASE + address_offset;

            opcode_address[0] = opcode;             // OPC read_address   [4]
            opcode_address[1] = lsb(read_address);  //
            opcode_address[2] = msb(read_address);  //
            opcode_address[3] = 0x60;               // RTS                [-]

            predicted_cycles = 4;

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(opcode_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, address_offset,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}

void test_read_abs_x_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned address_offset, reg_x;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

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

                predicted_cycles = 2 + 4 + different_pages(base_address, base_address + reg_x);

                dma_and_interrupts_off();
                actual_cycles = measure_cycles(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x X %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, address_offset, reg_x,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_read_abs_y_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned address_offset, reg_y;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

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

                predicted_cycles = 2 + 4 + different_pages(base_address, base_address + reg_y);

                dma_and_interrupts_off();
                actual_cycles = measure_cycles(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x Y %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, address_offset, reg_y,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_write_zpage_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned zp_address;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u zp addresses ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            opcode_address[0] = opcode;     // OPC zp_address   [3]
            opcode_address[1] = zp_address; //
            opcode_address[2] = 0x60;       // RTS              [-]

            predicted_cycles = 3;

            dma_and_interrupts_off();
            actual_cycles = measure_cycles_zp_safe(opcode_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x zp address %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, zp_address,
                       opcode_offset, actual_cycles);
            }
        }
    }
}

void test_write_zpage_x_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned zp_address, reg_x;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u zp addresses ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                opcode_address[0] = 0xa2;       // LDX #imm             [2]
                opcode_address[1] = reg_x;      //
                opcode_address[2] = opcode;     // OPC zp_address,X     [4]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS                  [1]

                predicted_cycles = 2 + 4;

                dma_and_interrupts_off();
                actual_cycles = measure_cycles_zp_safe(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x zp address %02x X %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, zp_address, reg_x,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_write_zpage_y_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned zp_address, reg_y;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u zp addresses ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_y = 0; reg_y <= 0xff; reg_y += STEP_SIZE)
            {
                opcode_address[0] = 0xa0;       // LDY #imm          [2]
                opcode_address[1] = reg_y;      //
                opcode_address[2] = opcode;     // OPC zp_address,Y  [4]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS               [-]

                predicted_cycles = 2 + 4;

                dma_and_interrupts_off();
                actual_cycles = measure_cycles_zp_safe(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x zp address %02x Y %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, zp_address, reg_y,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_write_abs_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned address_offset;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * write_address = TESTCODE_BASE + address_offset;

            opcode_address[0] = opcode;             // OPC write_address   [4]
            opcode_address[1] = lsb(write_address); //
            opcode_address[2] = msb(write_address); //
            opcode_address[3] = 0x60;               // RTS                 [-]

            predicted_cycles = 4;

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(opcode_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, address_offset,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}

void test_write_abs_x_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned address_offset, reg_x;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

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

                predicted_cycles = 2 + 5;

                dma_and_interrupts_off();
                actual_cycles = measure_cycles(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x X %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, address_offset, reg_x,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_write_abs_y_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned address_offset, reg_y;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

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

                predicted_cycles = 2 + 5;

                dma_and_interrupts_off();
                actual_cycles = measure_cycles(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x Y %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, address_offset, reg_y,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_read_modify_write_zpage_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned zp_address;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u zp addresses ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            opcode_address[0] = opcode;     // OPC zp_address      [5]
            opcode_address[1] = zp_address; //
            opcode_address[2] = 0x60;       // RTS                 [-]

            predicted_cycles = 5;

            dma_and_interrupts_off();
            actual_cycles = measure_cycles_zp_safe(opcode_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x zp address %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, zp_address,
                       opcode_offset, actual_cycles);
            }
        }
    }
}

void test_read_modify_write_zpage_x_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned zp_address, reg_x;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u zp addresses ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (zp_address = 0; zp_address <= 0xff; zp_address += STEP_SIZE)
        {
            for (reg_x = 0; reg_x <= 0xff; reg_x += STEP_SIZE)
            {
                opcode_address[0] = 0xa2;       // LDX #imm            [2]
                opcode_address[1] = reg_x;      //
                opcode_address[2] = opcode;     // OPC zp_address,X    [6]
                opcode_address[3] = zp_address; //
                opcode_address[4] = 0x60;       // RTS                 [-]

                predicted_cycles = 2 + 6;

                dma_and_interrupts_off();
                actual_cycles = measure_cycles_zp_safe(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x zp address %02x X %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, zp_address, reg_x,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_read_modify_write_abs_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned address_offset;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

        opcode_address = TESTCODE_ANCHOR + opcode_offset;

        for (address_offset = 0; address_offset <= 0xff; address_offset += STEP_SIZE)
        {
            uint8_t * abs_address = TESTCODE_BASE + address_offset;

            opcode_address[0] = opcode;           // OPC abs_address    [6]
            opcode_address[1] = lsb(abs_address); //
            opcode_address[2] = msb(abs_address); //
            opcode_address[3] = 0x60;             // RTS                [-]

            predicted_cycles = 6;

            dma_and_interrupts_off();
            actual_cycles = measure_cycles(opcode_address);
            dma_and_interrupts_on();
            ++test_count;

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_offset, address_offset,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}

void test_read_modify_write_abs_x_instruction(char * test_description, uint8_t opcode)
{
    unsigned opcode_offset;
    unsigned address_offset, reg_x;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

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

                predicted_cycles = 2 + 7;

                dma_and_interrupts_off();
                actual_cycles = measure_cycles(opcode_address);
                dma_and_interrupts_on();
                ++test_count;

                if (actual_cycles != predicted_cycles)
                {
                    ++error_count;
                    printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x X %02x cycles p/a %u/%u\n",
                        error_count,
                        test_description,
                        opcode_offset, address_offset, reg_x,
                        predicted_cycles, actual_cycles);
                }
            }
        }
    }
}

void test_brk_instruction(char * test_description)
{
    unsigned opcode_offset;
    unsigned address_offset, reg_x;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *opcode_address;
    uint8_t *oldvec;

    printf("[%lu] INFO (%s) testing %u opcode offsets ...\n",
            error_count, test_description, 1 + 255 / STEP_SIZE);

    for (opcode_offset = 0; opcode_offset <= 0xff; opcode_offset += STEP_SIZE)
    {
        printf("[%lu] INFO (%s) opcode offset %02x, testing %u address offsets ...\n",
                error_count, test_description, opcode_offset, 1 + 255 / STEP_SIZE);

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
        opcode_address[9] = 0x40;                      // RTS          [-]

        predicted_cycles = 2 + 4 + 7 + PLATFORM_SPECIFIC_IRQ_OVERHEAD + 4 + 2;

        dma_and_interrupts_off();
        oldvec = set_irq_vector_address(opcode_address + 3);
        actual_cycles = measure_cycles(opcode_address);
        set_irq_vector_address(oldvec);
        dma_and_interrupts_on();
        ++test_count;

        if (actual_cycles != predicted_cycles)
        {
            ++error_count;
            printf("[%lu] ERROR (%s) opcode offset %02x address offset %02x X %02x cycles p/a %u/%u\n",
                error_count,
                test_description,
                opcode_offset, address_offset, reg_x,
                predicted_cycles, actual_cycles);
        }
    }
}

void test_branch_instructions(void)
{
    // Test branch instructions on the sign a.k.a. negative (N) flag.

    test_branch_taken    ("BPL, taken"    , 0x10, false);
    test_branch_not_taken("BPL, not taken", 0x10, true );
    test_branch_taken    ("BMI, taken"    , 0x30, true );
    test_branch_not_taken("BMI, not taken", 0x30, false);

    // Test branch instructions on the overflow (V) flag.

    test_branch_taken    ("BVC, taken"    , 0x50, false);
    test_branch_not_taken("BVC, not taken", 0x50, true );
    test_branch_taken    ("BVS, taken"    , 0x70, true );
    test_branch_not_taken("BVS, not taken", 0x70, false);

    // Test branch instructions on the carry (C) flag.

    test_branch_taken    ("BCC, taken"    , 0x90, false);
    test_branch_not_taken("BCC, not taken", 0x90, true );
    test_branch_taken    ("BCS, taken"    , 0xb0, true );
    test_branch_not_taken("BCS, not taken", 0xb0, false);

    // Test branch instructions on the zero (Z) flag.

    test_branch_taken    ("BNE, taken"    , 0xd0, false);
    test_branch_not_taken("BNE, not taken", 0xd0, true);
    test_branch_taken    ("BEQ, taken"    , 0xf0, true);
    test_branch_not_taken("BEQ, not taken", 0xf0, false);
}

void test_single_byte_two_cycle_implied_instructions(void)
{
    // Test timing of the 18 single-byte, 2-cycle "implied" instructions.

    // CLV, CLC, SEC

    test_single_byte_instruction_sequence("CLV", 0xb8, 2);
    test_single_byte_instruction_sequence("CLC", 0x18, 2);
    test_single_byte_instruction_sequence("SEC", 0x38, 2);

    // CLI, SEI
    //
    // Note: These change CPU the interrupt flag, which may be critical to guarantee the proper working
    // of the external environment. For that reason, we sandwich these between PHP/PLP instructions.

    test_three_byte_instruction_sequence("CLI, between PHP and PLP", 0x08, 0x58, 0x28, 3 + 2 + 4);
    test_three_byte_instruction_sequence("SEI, between PHP and PLP", 0x08, 0x78, 0x28, 3 + 2 + 4);

    // CLD, SED
    // Note: The "SED" is tested in the SED,CLD combination, to prevent that the test leaves decimal mode enabled.

    test_single_byte_instruction_sequence("CLD", 0xd8, 2);
    test_two_byte_instruction_sequence   ("SED, followed by CLD", 0xf8, 0xd8, 2 + 2);

    // INX, DEX
    // INY, DEY

    test_single_byte_instruction_sequence("DEY", 0x88, 2);
    test_single_byte_instruction_sequence("INY", 0xc8, 2);
    test_single_byte_instruction_sequence("DEX", 0xca, 2);
    test_single_byte_instruction_sequence("INX", 0xe8, 2);

    // TAY, TYA
    // TAX, TXA

    test_single_byte_instruction_sequence("TYA", 0x98, 2);
    test_single_byte_instruction_sequence("TAY", 0xa8, 2);
    test_single_byte_instruction_sequence("TXA", 0x8a, 2);
    test_single_byte_instruction_sequence("TAX", 0xaa, 2);

    // TSX, TXS

    test_single_byte_instruction_sequence("TSX", 0xba, 2);
    test_two_byte_instruction_sequence   ("TXS, preceded by TSX", 0xba, 0x9a, 2 + 2);

    // ASL, ROL, LSR, ROR on the accumulator register.

    test_single_byte_instruction_sequence("ASL", 0x0a, 2);
    test_single_byte_instruction_sequence("ROL", 0x2a, 2);
    test_single_byte_instruction_sequence("LSR", 0x4a, 2);
    test_single_byte_instruction_sequence("ROR", 0x6a, 2);

    // NOP

    test_single_byte_instruction_sequence("NOP", 0xea, 2);
}

void test_single_byte_stack_instructions(void)
{
    // Test the single-byte, 6-cycle push/pull stack instructions.
    //
    // TSX, PHA, TXS              The stack pointer is saved before, and restored after the instruction.

    test_three_byte_instruction_sequence("PHA, between TSX and TXS", 0xba, 0x48, 0x9a, 2 + 3 + 2);

    // TSX, PHP, TXS              The stack pointer is saved before, and restored after the instruction.

    test_three_byte_instruction_sequence("PHP, between TSX and TXS", 0xba, 0x08, 0x9a, 2 + 3 + 2);

    // PHA, PLA                   The value to be pulled is pushed immediately before.

    test_two_byte_instruction_sequence("PLA, preceded by PHA", 0x48, 0x68, 3 + 4);

    // PHP, PLP                   The value to be pulled is pushed immediately before.

    test_two_byte_instruction_sequence("PLP, preceded by PHP", 0x08, 0x28, 3 + 4);
}

void test_immediate_mode_instructions(void)
{
    // The 11 immediate-mode instructions all take 2 cycles.

    test_immediate_mode_instruction("LDY #imm", 0xa0);
    test_immediate_mode_instruction("LDX #imm", 0xa2);
    test_immediate_mode_instruction("CPY #imm", 0xc0);
    test_immediate_mode_instruction("CPX #imm", 0xe0);

    test_immediate_mode_instruction("ORA #imm", 0x09);
    test_immediate_mode_instruction("AND #imm", 0x29);
    test_immediate_mode_instruction("EOR #imm", 0x49);
    test_immediate_mode_instruction("ADC #imm", 0x69);
    test_immediate_mode_instruction("LDA #imm", 0xa9);
    test_immediate_mode_instruction("CMP #imm", 0xc9);
    test_immediate_mode_instruction("SBC #imm", 0xe9);
}

void test_read_zpage_instructions(void)
{
    // The 12 read-from-zero-page instructions all take 3 cycles.

    test_read_zpage_instruction("BIT zpage", 0x24);
    test_read_zpage_instruction("LDX zpage", 0xa6);
    test_read_zpage_instruction("LDY zpage", 0xa4);
    test_read_zpage_instruction("CPX zpage", 0xe4);
    test_read_zpage_instruction("CPY zpage", 0xc4);

    test_read_zpage_instruction("ORA zpage", 0x05);
    test_read_zpage_instruction("AND zpage", 0x25);
    test_read_zpage_instruction("EOR zpage", 0x45);
    test_read_zpage_instruction("ADC zpage", 0x65);
    test_read_zpage_instruction("LDA zpage", 0xa5);
    test_read_zpage_instruction("CMP zpage", 0xc5);
    test_read_zpage_instruction("SBC zpage", 0xe5);
}

void test_read_zpage_x_instructions(void)
{
    // The 8 read-from-zero-page-with-x-indexing instructions all take 4 cycles.

    test_read_zpage_x_instruction("LDY zpage,X", 0xb4);

    test_read_zpage_x_instruction("ORA zpage,X", 0x15);
    test_read_zpage_x_instruction("AND zpage,X", 0x35);
    test_read_zpage_x_instruction("EOR zpage,X", 0x55);
    test_read_zpage_x_instruction("ADC zpage,X", 0x75);
    test_read_zpage_x_instruction("LDA zpage,X", 0xb5);
    test_read_zpage_x_instruction("CMP zpage,X", 0xd5);
    test_read_zpage_x_instruction("SBC zpage,X", 0xf5);
}

void test_read_zpage_y_instructions(void)
{
    // The 1 read-from-zero-page-with-y-indexing instruction takes 4 cycles.

    test_read_zpage_y_instruction("LDX zpage,Y", 0xb6);
}

void test_read_abs_instructions(void)
{
    // The 12 read-from-absolute-address instructions all take 4 cycles.

    test_read_abs_instruction("BIT abs", 0x2c);
    test_read_abs_instruction("LDX abs", 0xae);
    test_read_abs_instruction("LDY abs", 0xac);
    test_read_abs_instruction("CPX abs", 0xec);
    test_read_abs_instruction("CPY abs", 0xcc);

    test_read_abs_instruction("ORA abs", 0x0d);
    test_read_abs_instruction("AND abs", 0x2d);
    test_read_abs_instruction("EOR abs", 0x4d);
    test_read_abs_instruction("ADC abs", 0x6d);
    test_read_abs_instruction("LDA abs", 0xad);
    test_read_abs_instruction("CMP abs", 0xcd);
    test_read_abs_instruction("SBC abs", 0xed);
}

void test_read_abs_x_instructions(void)
{
    // The 8 read-from-absolute-addressing-with-x-indexing instructions take 4 or 5 cycles.
    // An extra cycle is added in case the indexing with X causes the effective address
    // to be on a different page than the base address.

    test_read_abs_x_instruction("LDY abs,X", 0xbc);

    test_read_abs_x_instruction("ORA abs,X", 0x1d);
    test_read_abs_x_instruction("AND abs,X", 0x3d);
    test_read_abs_x_instruction("EOR abs,X", 0x5d);
    test_read_abs_x_instruction("ADC abs,X", 0x7d);
    test_read_abs_x_instruction("LDA abs,X", 0xbd);
    test_read_abs_x_instruction("CMP abs,X", 0xdd);
    test_read_abs_x_instruction("SBC abs,X", 0xfd);
}

void test_read_abs_y_instructions(void)
{
    // The 8 read-from-absolute-addressing-with-y-indexing instructions take 4 or 5 cycles.
    // An extra cycle is added in case the indexing with Y causes the effective address
    // to be on a different page than the base address.

    test_read_abs_y_instruction("LDX abs,Y", 0xbe);

    test_read_abs_y_instruction("ORA abs,Y", 0x19);
    test_read_abs_y_instruction("AND abs,Y", 0x39);
    test_read_abs_y_instruction("EOR abs,Y", 0x59);
    test_read_abs_y_instruction("ADC abs,Y", 0x79);
    test_read_abs_y_instruction("LDA abs,Y", 0xb9);
    test_read_abs_y_instruction("CMP abs,Y", 0xd9);
    test_read_abs_y_instruction("SBC abs,Y", 0xf9);
}

void test_write_zpage_instructions(void)
{
    // The 3 write-to-zero-page instructions all take 3 cycles.

    test_write_zpage_instruction("STA zpage", 0x85);
    test_write_zpage_instruction("STX zpage", 0x86);
    test_write_zpage_instruction("STY zpage", 0x84);
}

void test_write_zpage_x_instructions(void)
{
    // The 3 write-to-zero-page-with-x-indexing instructions all take 4 cycles.

    test_write_zpage_x_instruction("STA zpage,X", 0x95);
    test_write_zpage_x_instruction("STY zpage,X", 0x94);
}

void test_write_zpage_y_instructions(void)
{
    // The 3 write-to-zero-page-with-y-indexing instructions all take 4 cycles.

    test_write_zpage_y_instruction("STX zpage,Y", 0x96);
}

void test_write_abs_instructions(void)
{
    // The 3 write-to-absolute-address instructions all take 4 cycles.

    test_write_abs_instruction("STA abs", 0x8d);
    test_write_abs_instruction("STX abs", 0x8e);
    test_write_abs_instruction("STY abs", 0x8c);
}

void test_write_abs_x_instructions(void)
{
    // The 1 write-to-absolute-address-with-x-indexing instructions all take 5 cycles.

    test_write_abs_x_instruction("STA abs,X", 0x9d);
}

void test_write_abs_y_instructions(void)
{
    // The 1 write-to-absolute-address-with-y-indexing instructions all take 5 cycles.

    test_write_abs_y_instruction("STA abs,Y", 0x99);
}

void test_read_modify_write_zpage_instructions(void)
{
    // The 6 read-modify-write-to-zero-page instructions all take 5 cycles.

    test_read_modify_write_zpage_instruction("ASL zpage", 0x06);
    test_read_modify_write_zpage_instruction("ROL zpage", 0x26);
    test_read_modify_write_zpage_instruction("LSR zpage", 0x46);
    test_read_modify_write_zpage_instruction("ROR zpage", 0x66);
    test_read_modify_write_zpage_instruction("DEC zpage", 0xc6);
    test_read_modify_write_zpage_instruction("INC zpage", 0xe6);
}

void test_read_modify_write_zpage_x_instructions(void)
{
    // The 6 read-modify-write-to-zero-page instructions all take 6 cycles.

    test_read_modify_write_zpage_x_instruction("ASL zpage,X", 0x16);
    test_read_modify_write_zpage_x_instruction("ROL zpage,X", 0x36);
    test_read_modify_write_zpage_x_instruction("LSR zpage,X", 0x56);
    test_read_modify_write_zpage_x_instruction("ROR zpage,X", 0x76);
    test_read_modify_write_zpage_x_instruction("DEC zpage,X", 0xd6);
    test_read_modify_write_zpage_x_instruction("INC zpage,X", 0xf6);
}

void test_read_modify_write_abs_instructions(void)
{
    // The 6 read-modify-write-to-absolute-address instructions all take 6 cycles.

    test_read_modify_write_abs_instruction("ASL abs", 0x0e);
    test_read_modify_write_abs_instruction("ROL abs", 0x2e);
    test_read_modify_write_abs_instruction("LSR abs", 0x4e);
    test_read_modify_write_abs_instruction("ROR abs", 0x6e);
    test_read_modify_write_abs_instruction("DEC abs", 0xce);
    test_read_modify_write_abs_instruction("INC abs", 0xee);
}

void test_read_modify_write_abs_x_instructions(void)
{
    // The 6 read-modify-write-to-absolute-address instructions all take 7 cycles.

    test_read_modify_write_abs_x_instruction("ASL abs,X", 0x1e);
    test_read_modify_write_abs_x_instruction("ROL abs,X", 0x3e);
    test_read_modify_write_abs_x_instruction("LSR abs,X", 0x5e);
    test_read_modify_write_abs_x_instruction("ROR abs,X", 0x7e);
    test_read_modify_write_abs_x_instruction("DEC abs,X", 0xde);
    test_read_modify_write_abs_x_instruction("INC abs,X", 0xfe);
}

void run_cpu_test(void)
{
    // Test the timing of the 8 branch instructions.
    test_branch_instructions();

    // Test the timing of the 22 single-byte, two-cycle opcodes.
    test_single_byte_two_cycle_implied_instructions();

    // Test the timing of the 4 stack push/pull instructions.
    test_single_byte_stack_instructions();

    // Test the timing of the 11 two-byte immediate-mode instructions.
    test_immediate_mode_instructions();

    // Test the timing of the 12 two-byte read-from-zero-page instructions.
    test_read_zpage_instructions();

    // Test the timing of the 8 two-byte read-from-zero-page-with-x-indexing instructions.
     test_read_zpage_x_instructions();

    // Test the timing of the 1 two-byte read-from-zero-page-with-y-indexing instruction.
     test_read_zpage_y_instructions();

    // Test the timing of the 12 two-byte read-from-abs-address instructions.
     test_read_abs_instructions();

    // Test the timing of the 8 three-byte read-from-zero-page-with-x-indexing instructions.
     test_read_abs_x_instructions();

    // Test the timing of the 8 three-byte read-from-zero-page-with-y-indexing instructions.
     test_read_abs_y_instructions();

    // Test the timing of the 3 two-byte write-to-zero-page instructions.
    test_write_zpage_instructions();

    // Test the timing of the 2 two-byte write-to-zero-page-with-x-indexing instructions.
    test_write_zpage_x_instructions();

    // Test the timing of the 1 two-byte write-to-zero-page-with-y-indexing instruction.
    test_write_zpage_y_instructions();

    // Test the timing of the 3 three-byte write-to-absolute-address instructions.
     test_write_abs_instructions();

    // Test the timing of the 1 three-byte write-to-absolute-address-with-x-indexing instruction.
    test_write_abs_x_instructions();

    // Test the timing of the 1 three-byte write-to-absolute-address-with-y-indexing instruction.
    test_write_abs_y_instructions();

    // Test the timing of the 6 two-byte read-modify-write-zpage instructions.
    test_read_modify_write_zpage_instructions();

    // Test the timing of the 6 two-byte read-modify-write-zpage-with-x-indexing instructions.
    test_read_modify_write_zpage_x_instructions();

    // Test the timing of the 6 three-byte read-modify-write-absolute-address instructions.
    test_read_modify_write_abs_instructions();

    // Test the timing of the 6 three-byte read-modify-write-absolute-address-with-x-indexing instructions.
    test_read_modify_write_abs_x_instructions();

    // So far, tests were implemented for 129/151 instructions; 22 remaining.
    //
    // =============================================================== TODO: Read (ind,X)         (7)      7
    //
    // ORA, AND, EOR, ADC, LDA, CMP, SBC
    //
    // =============================================================== TODO: Read (ind),Y         (7)      14
    //
    // ORA, AND, EOR, ADC, LDA, CMP, SBC
    //
    // =============================================================== TODO: Write (ind,X)        (1)      15
    //
    // STA
    //
    // =============================================================== TODO: Write (ind),Y        (1)      16
    //
    // STA
    //
    // =============================================================== TODO: Misc instructions    (6)      22
    //
    // JSR abs          JSR, followed by two PLAs and an RTS.
    // RTI              Push PHP and a return address.  3/2/3/2/3/RTI
    // RTS              Push a return address, then execute the return. 2/3/2/3/RTS
    // JMP abs          Just execute the JMP.
    // JMP (ind)
    // BRK              Reroute OS IRQ handler. Execute BRK/NOP/RTS/RTI
}

int command_line_loop(void)
{
    char command[80];
    unsigned par1, par2, par3;
    int result;

    result = allocate_testcode_block(4096);
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
    printf("  TESTCODE_END     %p\n", TESTCODE_BASE + TESTCODE_SIZE);
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

            printf("\n");
            printf("Tests performed ...... : %lu\n", test_count);
            printf("Tests failed ......... : %lu\n", error_count);
            printf("\n");
        }
        else if (sscanf(command, "cpu %u", &par1) == 1)
        {
            const unsigned lookup_table[8] = {1, 3, 5, 165, 17, 51, 85, 255};

            printf("\n");

            if (par1 > 7)
            {
                printf("Level must be 0 to 7. Higher values\n");
                printf("test more cases, but are MUCH slower.\n");
                printf("\n");
                continue;
            }

            STEP_SIZE = lookup_table[7 - par1];

            printf("Starting test with step size %u.\n", STEP_SIZE);
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
            printf("  * level: 1 (fast) to 7 (slow)\n");
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

    printf("*** TIC v0.1.5 ***\n");
    printf("\n");

    result = command_line_loop();

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
