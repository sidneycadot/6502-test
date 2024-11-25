
////////////////////////////////
// tic_cmd_measurement_test.c //
////////////////////////////////

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "tic_cmd_measurement_test.h"
#include "timing_test_report.h"
#include "timing_test_memory.h"
#include "measure_cycles.h"


static void generate_code(uint8_t * code, unsigned cycles)
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

void tic_cmd_measurement_test(unsigned repeats, unsigned min_cycle_count, unsigned max_cycle_count)
{
    unsigned instruction_cycles, actual_cycles, rep;

    printf("Performing measurement tests ...\n");

    reset_test_counts();

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
    report_test_counts();
}
