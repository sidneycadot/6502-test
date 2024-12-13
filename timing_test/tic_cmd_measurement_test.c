
////////////////////////////////
// tic_cmd_measurement_test.c //
////////////////////////////////

#include <stdio.h>
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

static bool run_measurement_tests(unsigned repeats, uint8_t min_cycle_count, uint8_t max_cycle_count)
{
    // Generate straightforward 6502 code to burn a desired number of instruction cycles, then
    // execute the 'measure_cycles' routine on the genrated code to verify that the number of cycles
    // measured is equal to the number of cycles the code was expected to take.

    unsigned repeat_index;

    extern unsigned m_test_overhead_cycles;
    extern unsigned m_instruction_cycles;

    for (repeat_index = 1; repeat_index <= repeats; ++repeat_index)
    {
        pre_every_test_hook("measurement test");

        parspec = Par1_ClockCycleCount;

        for (par1 = min_cycle_count;; ++par1)
        {
            if (par1 == 1)
            {
                // Cannot generate 1-cycle test code.
                continue;
            }

            generate_code(TESTCODE_BASE, par1);

            m_test_overhead_cycles = 0;
            m_instruction_cycles = par1;

            // Note that we do not bail out in case of errors.
            if (!run_measurement("measurement test", TESTCODE_BASE, F_NONE))
                return false;

            if (par1 == max_cycle_count)
                break;
        }
    }
    return true;
}

void tic_cmd_measurement_test(unsigned repeats, uint8_t min_cycle_count, uint8_t max_cycle_count)
{
    // This command runs a test on the time measurement code itself.

    reset_test_counts();
    pre_big_measurement_block_hook();
    run_measurement_tests(repeats, min_cycle_count, max_cycle_count);
    post_big_measurement_block_hook();
    report_test_counts();
}
