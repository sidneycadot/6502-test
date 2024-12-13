
///////////////////////////
// target_c64_specific.c //
///////////////////////////

#include <stdbool.h>
#include <peekpoke.h>

#include "target.h"

void program_start_hook(void)
{
}

void program_end_hook(void)
{
}

void pre_every_test_hook(const char * opcode_description)
{
    (void)opcode_description;
}

bool post_every_measurement_hook(bool success, unsigned opcode_count, unsigned long measurement_count, unsigned long error_count)
{
    (void)success;
    (void)opcode_count;
    (void)measurement_count;
    (void)error_count;
    POKE(0xd020, measurement_count);

    return true; // Continue (do not cancel) the run.
}
