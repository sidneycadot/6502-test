
////////////////////////////////
// tic_cmd_measurement_test.c //
////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "tic_cmd_measurement_test.h"
#include "timing_test_measurement.h"
#include "timing_test_memory.h"
#include "target.h"

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
            while (!zp_address_is_safe_for_read(zp_address))
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

static bool run_measurement_tests(unsigned repeats, unsigned min_cycle_count, unsigned max_cycle_count)
{
    // Generate straightforward 6502 code to burn a desired number of instruction cycles, then
    // execute the 'measure_cycles' routine on the genrated code to verify that the number of cycles
    // measured is equal to the number of cycles the code was expected to take.

    unsigned repeat_index;
    unsigned cycle_count;

    prepare_opcode_tests("SLEEP", Par1234_Generic);

    for (repeat_index = 1; repeat_index <= repeats; ++repeat_index)
    {
        par1 = repeat_index % 256;
        par2 = repeat_index / 256;

        for (cycle_count = min_cycle_count; cycle_count <= max_cycle_count; ++cycle_count)
        {
            if (cycle_count == 1)
            {
                // Cannot generate 1-cycle test code.
                continue;
            }

            par3 = cycle_count % 256;
            par4 = cycle_count / 256;

            generate_code(TESTCODE_BASE, cycle_count);

            m_test_overhead_cycles = 0;
            m_instruction_cycles = cycle_count;

            // Note that we do not bail out in case of errors.
            if (!execute_single_opcode_test(TESTCODE_BASE, F_NONE))
                return false;
        }
    }
    return true;
}

void tic_cmd_measurement_test(unsigned repeats, unsigned min_cycle_count, unsigned max_cycle_count)
{
    // This command runs a test on the time measurement code itself.
    reset_test_counts();
    pre_big_measurement_block_hook();
    run_measurement_tests(repeats, min_cycle_count, max_cycle_count);
    post_big_measurement_block_hook();
    printf("\n");
    report_test_counts();
}
