
//////////////////
// target_c64.c //
//////////////////

#include <stdbool.h>
#include <peekpoke.h>

#include "target.h"

void program_start_hook(void)
{
}

void program_end_hook(void)
{
}

void pre_every_test_hook(const char * test_description)
{
}

void post_every_measurement_hook(const char * test_description, bool success, unsigned long test_count, unsigned long error_count)
{
    (void)test_description;
    (void)success;
    (void)error_count;
    POKE(0xd020, test_count);
}
