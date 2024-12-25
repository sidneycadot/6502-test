
///////////////////////////
// target_c64_specific.c //
///////////////////////////

#include <stdbool.h>
#include <peekpoke.h>
#include <stdio.h>

#include "target.h"

#define VIC_BORDERCOLOR 0xd020

void program_start_hook(void)
{
}

void program_end_hook(void)
{
}

void pre_opcode_hook(const char * opcode_description, bool skip_flag)
{
    (void)opcode_description;
    (void)skip_flag;
}

bool post_every_measurement_hook(bool success, unsigned opcode_count, unsigned long measurement_count, unsigned long error_count)
{
    (void)success;
    (void)opcode_count;
    (void)measurement_count;
    (void)error_count;
    POKE(VIC_BORDERCOLOR, measurement_count);

    return true; // Continue (do not cancel) the run.
}
