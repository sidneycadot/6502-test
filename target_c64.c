
//////////////////
// target_c64.c //
//////////////////

#include <stdbool.h>
#include <peekpoke.h>

#include "target.h"

void measurement_live_report(const char * test_description, unsigned long test_count, bool success)
{
    (void)test_description;
    (void)success;
    POKE(0xd020, test_count);
}
