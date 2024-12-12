
////////////////////////
// tic_cmd_cpu_test.c //
////////////////////////

#include <stdio.h>

#include "timing_test_routines.h"
#include "timing_test_measurement.h"
#include "target.h"

#include "tic_cmd_cpu_test.h"

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

    //run_completed = run_6502_instruction_timing_tests();
    run_completed = timing_test_buggy_6502_illegal_instructions();

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
