
////////////////
// tic_main.c //
////////////////

// This program tests the instruction timing (i.e., clock-cycle counts) of
// the 151 documented 6502 instructions.
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
// which may be in use by the 6502 machine's operating system. This function depends on the
// platform dependent measure_cycles() routine, but is itself platform independent. It is a
// drop-in replacement for the standard 'measure_cycles' for cases when this zero-page
// preservation behavior is needed:
//
// * measure_cycles_zp_safe()   Save the contents of the zero page, execute measure_cycles(),
//                              then restore the content of the zero page.
//
// ASSUMPTIONS
// -----------
//
// * The program assumes that the 6502 instructions work correctly, other than that their clock
//   cycle counts may be off.
//
// * Later tests assume that the clock cycle counts of 6502 instructions that were timed
//   by earlier tests for simpler instructions are correct.
//
// * The 65C02 changes (fixes) the behavior of the C and V flags when doing addition or subtraction
//   (ADC or SBC) in decimal mode, at the cost of an extra clock cycle. This will currently not
//   be tested, as the tests are all run with decimal mode disabled. We will add explicit tests
//   for this once we have a 65C02 system available.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timing_test_memory.h"

#include "tic_cmd_measurement_test.h"
#include "tic_cmd_cpu_test.h"

void tic_cmd_help(void)
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
    printf("  * level: 0 (fast) to 7 (slow)\n");
    printf("\n");
    printf("> quit\n");
    printf("\n");
    printf("  Quit the program.\n");
    printf("\n");
}

int command_line_loop(void)
{
    int result;

    result = allocate_testcode_block(2048);
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
    printf("  TESTCODE_LAST    %p\n", TESTCODE_LAST);
    printf("\n");

    for (;;)
    {
        char command[80];
        unsigned par1, par2, par3;

        printf("Enter command (or ENTER for help)\n");
        fgets(command, sizeof(command), stdin);

        if (strcmp(command, "quit") == 0)
        {
            break;
        }
        else if (sscanf(command, "msm %u %u %u", &par1, &par2, &par3) == 3)
        {
            tic_cmd_measurement_test(par1, par2, par3);
        }
        else if (sscanf(command, "cpu %u", &par1) == 1)
        {
            tic_cmd_cpu_test(par1);
        }
        else
        {
            tic_cmd_help();
        }
    }

    free_testcode_block();

    return 0; // Report success.
}

int main(void)
{
    int result;

    printf("*** TIC v0.2.2 ***\n");

    //printf("\n");
    result = command_line_loop();

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
