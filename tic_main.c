
////////////////
// tic_main.c //
////////////////

// This program tests the instruction timing (i.e., clock-cycle counts) of
// the 151 documented 6502 instructions.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

        // Remove the trailing end-of-line character.

        par1 = strlen(command);
        if (par1 != 0)
        {
            command[par1 - 1] = '\0';
        }

        // Check for a command.

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

    printf("*** TIC v0.2.4 ***\n");
    printf("\n");

    result = command_line_loop();

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
