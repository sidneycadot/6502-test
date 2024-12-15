
////////////////
// tic_main.c //
////////////////

// This program tests the instruction timing (i.e., clock-cycle counts) of 6502/65C02 instructions.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timing_test_memory.h"
#include "tic_cmd_measurement_test.h"
#include "tic_cmd_cpu_test.h"
#include "target.h"

uint8_t cpu_signature;

void tic_cmd_help(void)
{
    printf("Commands:\n");
    printf("\n");
    printf("> msm <nreps> <min_c> <max_c>\n");
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

    for (;;)
    {
        char command[80];
        unsigned par1, par2, par3;

        printf("Enter command (or ENTER for help):\n");
        printf("\n");
        fgets(command, sizeof(command), stdin);

        // Remove the trailing end-of-line character.

        par1 = strlen(command);
        if (par1 != 0)
        {
            command[par1 - 1] = '\0';
        }

        if (strlen(command) != 0)
        {
            printf("\n");
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
        else if (sscanf(command, "msm %u %u", &par1, &par2) == 2)
        {
            tic_cmd_measurement_test(par1, par2, par2);
        }
        else if (sscanf(command, "msm %u", &par1) == 1)
        {
            tic_cmd_measurement_test(1, par1, par1);
        }
        else if (sscanf(command, "cpu %u", &par1) == 1)
        {
            tic_cmd_cpu_test(par1);
        }
        else
        {
            // No valid command found, show help.
            tic_cmd_help();
        }
    }

    free_testcode_block();

    return 0; // Report success.
}

const char * cpu_signature_string[4] = {
    "6502 with decimal mode",
    "6502 without decimal mode",
    "65C02",
    "unknown"
};

int main(void)
{
    int result;

    cpu_signature = get_cpu_signature();

    program_start_hook();

    printf("*** TIC v0.5.1 ***\n");
    printf("\n");
    printf("CPU signature:\n\n  0x%02x: %s.\n\n", cpu_signature, cpu_signature_string[cpu_signature]);

    result = command_line_loop();

    program_end_hook();

    return (result == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
