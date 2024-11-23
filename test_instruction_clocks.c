
///////////////////////////
// test_measure_cycles.c //
///////////////////////////

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "measure_cycles.h"

uint8_t * TESTCODE_PTR    = NULL; // The pointer, allocated using malloc().
uint8_t * TESTCODE_BASE   = NULL; // The first address in the TESTCODE range that is on a page boundary.
uint8_t * TESTCODE_ANCHOR = NULL; // The halfway point in the TESTCODE range. Put test code here.
unsigned  TESTCODE_SIZE   = 0;

unsigned long test_count;
unsigned long error_count;

static int allocate_testcode_block(unsigned size)
{
    unsigned offset;

    if (size % 2 != 0)
    {
        return -1;
    }

    // We want a paged-aligned block of size 'size'. To guarantee that, we'll allocate 255 bytes more.
    TESTCODE_PTR = malloc(size + 255);
    if (TESTCODE_PTR == NULL)
    {
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

static void free_testcode_block(void)
{
    free (TESTCODE_PTR);
}

static void generate_code(uint8_t * code, unsigned cycles)
{
    assert (cycles != 1);

    while (cycles != 0)
    {
        if (cycles % 2 != 0)
        {
            *code++ = 0xa5; // insert LDA 0 (zp)
            *code++ = 0x00;
            cycles -= 3;
        }
        else
        {
            *code++ = 0xea; // insert NOP
            cycles -= 2;
        }
    }
    *code++ = 0x60; // RTS
}

void baseline_test(void)
{
    unsigned k, cycles;

    dma_and_interrupts_off();

    for (k = 0; k <= 28; ++k)
    {
        if (k == 1)
        {
            // Cannot generate 1-cycle code.
            continue;
        }

        generate_code(TESTCODE_ANCHOR, k);

        cycles = measure_cycles(TESTCODE_ANCHOR);

        printf("cycles: %u -> %d%s\n", k, cycles, (k == cycles) ? "" : " *** ERROR ***");
    }

    dma_and_interrupts_on();
}

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

static void test_branch_taken(char * test_description, uint8_t branch_opcode, uint8_t lda_value, uint8_t adc_value)
{
    // This function tests any of the "branch" instructions, assuming the flags are in a state that lead to the branch being taken.
    // The following instructions are performed before executing the branch instruction:
    //
    //     CLC
    //     LDA  #<lda_value>
    //     ADC  #<adc_value>
    //     JMP  branch_opcode_address
    //
    // By picking suitable values for 'lda_value' and 'adc_value', each of the N/V/C/Z flags can be initialized to a value that is
    // suitable for the test at hand; specifically, to ensure that the branch instruction to be tested is taken.
    //
    // Branch instructions, when taken, take 3 or 4 clock cycles:
    //
    // * 3 clock cycles if the address following the branch instruction is on the same memory page as the destination address;
    // * 4 clock cycles if the address following the branch instruction is *not* on the same memory page as the destination address.

    unsigned opcode_page_offset;
    unsigned operand;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *entry_address;
    uint8_t *branch_opcode_address;

    for (opcode_page_offset = 0; opcode_page_offset <= 0xff; ++opcode_page_offset)
    {
        printf("[%lu] INFO (%s) page offset %02u, testing 256 operands ...\n",
                error_count, test_description, opcode_page_offset);

        entry_address = TESTCODE_BASE;
        branch_opcode_address = TESTCODE_ANCHOR + opcode_page_offset;

        for (operand = 0; operand <= 0xff; ++operand)
        {
            int displacement = (operand <= 0x7f) ? operand : operand - 0x100;

            if (displacement == -1 || displacement == -2)
            {
                // Putting an RTS here would overwrite the branch instruction itself; skip.
                continue;
            }

            entry_address[0] = 0x18;
            entry_address[1] = 0xa9;
            entry_address[2] = lda_value;
            entry_address[3] = 0x69;
            entry_address[4] = adc_value;
            entry_address[5] = 0x4c;
            entry_address[6] = lsb(branch_opcode_address);
            entry_address[7] = msb(branch_opcode_address);

            branch_opcode_address[0] = branch_opcode;
            branch_opcode_address[1] = operand;
            branch_opcode_address[2 + displacement] = 0x60; // rts.

            predicted_cycles = 2 + 2 + 2 + 3 + 3 + different_pages(&branch_opcode_address[2], &branch_opcode_address[2 + displacement]);

            dma_and_interrupts_off();
            ++test_count;
            actual_cycles = measure_cycles(entry_address);
            dma_and_interrupts_on();

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) offset %02x operand %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_page_offset, operand,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}


static void test_branch_not_taken(char * test_description, uint8_t branch_opcode, uint8_t load_value, uint8_t add_value)
{
    // This function tests any of the "branch" instructions, assuming the flags are in a state that lead to the branch *not* being taken.
    // The following instructions are performed before executing the branch instruction:
    //
    //     CLC
    //     LDA  #<lda_value>
    //     ADC  #<adc_value>
    //     JMP  branch_opcode_address
    //
    // By picking suitable values for 'lda_value' and 'adc_value', each of the N/V/C/Z flags can be initialized to a value that is
    // suitable for the test at hand; specifically, to ensure that the branch instruction to be tested is *not* taken.
    //
    // Branch instructions, when not taken, always take 2 clock cycles.
    //

    unsigned opcode_page_offset;
    unsigned operand;
    unsigned predicted_cycles, actual_cycles;
    uint8_t *entry_address;
    uint8_t *branch_opcode_address;

    for (opcode_page_offset = 0; opcode_page_offset <= 0xff; ++opcode_page_offset)
    {
        printf("[%lu] INFO (%s) page offset %02u, testing 256 operands ...\n",
                error_count, test_description, opcode_page_offset);

        entry_address = TESTCODE_BASE;
        branch_opcode_address = TESTCODE_ANCHOR + opcode_page_offset;

        for (operand = 0; operand <= 0xff; ++operand)
        {
            entry_address[0] = 0x18;
            entry_address[1] = 0xa9;
            entry_address[2] = load_value;
            entry_address[3] = 0x69;
            entry_address[4] = add_value;
            entry_address[5] = 0x4c;
            entry_address[6] = lsb(branch_opcode_address);
            entry_address[7] = msb(branch_opcode_address);

            branch_opcode_address[0] = branch_opcode;
            branch_opcode_address[1] = operand;
            branch_opcode_address[2] = 0x60; // rts.

            predicted_cycles = 2 + 2 + 2 + 3 + 2;

            dma_and_interrupts_off();
            ++test_count;
            actual_cycles = measure_cycles(entry_address);
            dma_and_interrupts_on();

            if (actual_cycles != predicted_cycles)
            {
                ++error_count;
                printf("[%lu] ERROR (%s) offset %02x operand %02x cycles p/a %u/%u\n",
                       error_count,
                       test_description,
                       opcode_page_offset, operand,
                       predicted_cycles, actual_cycles);
            }
        }
    }
}

void test_branch_instructions(void)
{
    // Test branch instructions on the sign (N) flag.

    test_branch_taken    ("BPL, taken"    , 0x10, 0x00, 0x00);
    test_branch_not_taken("BPL, not taken", 0x10, 0x80, 0x00);
    test_branch_taken    ("BMI, taken"    , 0x30, 0x80, 0x00);
    test_branch_not_taken("BMI, not taken", 0x30, 0x00, 0x00);

    // Test branch instructions on the overflow (V) flag.

    test_branch_taken    ("BVC, taken"    , 0x50, 0x00, 0x00);
    test_branch_taken    ("BVC, not taken", 0x50, 0x40, 0x40);
    test_branch_taken    ("BVS, taken"    , 0x70, 0x40, 0x40);
    test_branch_taken    ("BVS, not taken", 0x70, 0x00, 0x00);

    // Test branch instructions on the carry (C) flag.

    test_branch_taken    ("BCC, taken"    , 0x90, 0x00, 0x00);
    test_branch_not_taken("BCC, not taken", 0x90, 0x80, 0x80);
    test_branch_taken    ("BCS, taken"    , 0xb0, 0x80, 0x80);
    test_branch_not_taken("BCS, not taken", 0xb0, 0x00, 0x00);

    // Test branch instructions on the zero (Z) flag.

    test_branch_taken    ("BNE, taken"    , 0xd0, 0xff, 0x00);
    test_branch_not_taken("BNE, not taken", 0xd0, 0x00, 0x00);
    test_branch_taken    ("BEQ, taken"    , 0xf0, 0x00, 0x00);
    test_branch_not_taken("BEQ, not taken", 0xf0, 0xff, 0x00);
}

int main(void)
{
    int result;

    printf("*** TIC v0.1.0 ***\n");
    printf("\n");

    result = allocate_testcode_block(4096);
    if (result != 0)
    {
        puts("Unable to allocate TESTCODE block.");
        return EXIT_FAILURE;
    }

    printf("Test memory was allocated as follows:\n");
    printf("\n");
    printf("  TESTCODE_PTR     %p\n", TESTCODE_PTR);
    printf("  TESTCODE_BASE    %p\n", TESTCODE_BASE);
    printf("  TESTCODE_ANCHOR  %p\n", TESTCODE_ANCHOR);
    printf("\n");

    for (;;)
    {
        printf("Press ENTER to start testing.\n");
        getchar();

        test_count = 0;
        error_count = 0;
 
        // This tests the timing of the 8 instructions {BPL, BMI, BVC, BVS, BCC, BCS, BNE, BEQ}.
        test_branch_instructions();

        printf("\n");
        printf("Tests performed ...... : %lu.\n", test_count);
        printf("Tests failed ......... : %lu.\n", error_count);
        printf("\n");
    }

    free_testcode_block();

    return EXIT_SUCCESS;
}
