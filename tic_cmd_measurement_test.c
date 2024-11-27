
////////////////////////////////
// tic_cmd_measurement_test.c //
////////////////////////////////

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "tic_cmd_measurement_test.h"
#include "timing_test_report.h"
#include "timing_test_memory.h"
#include "platform.h"

static void generate_code(uint8_t * code, unsigned cycles)
{
    // Generate simple code starting at the pointer 'code' that will
    // burn 'cycles' clock cycles, then perform an RTS.

    assert(cycles != 1);

    while (cycles != 0)
    {
        if (cycles % 2 != 0)
        {
            // Insert a three-cycle opcode: load from zero page.
            // First, Find a safe zero-page address to load from.
            uint8_t zp_address = 0x00;
            while (!zp_address_is_safe(zp_address))
            {
                ++zp_address;
            }

            *code++ = 0xa5;         // LDA zp_address  [3]
            *code++ = zp_address;
            cycles -= 3;
        }
        else
        {
            *code++ = 0xea;         // NOP             [2]
            cycles -= 2;
        }
    }
    *code++ = 0x60;                 // RTS             [-]
}

void tic_cmd_measurement_test(unsigned repeats, unsigned min_cycle_count, unsigned max_cycle_count)
{
    // This command runs a test on the time measurement code itself.
    // It does this by generating straightforward 6502 code to burn a desired number of instruction
    // cycles, then executing the 'measure_cycles' routine to verify that the number of cycles measured
    // is equal to the number of cycles the code was expected to take.

    unsigned instruction_cycles, actual_cycles, repeat_index;

    printf("Performing measurement tests ...\n");

    reset_test_counts();

    for (repeat_index = 1; repeat_index <= repeats; ++repeat_index)
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
    report_test_counts();
}
