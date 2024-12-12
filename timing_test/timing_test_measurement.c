
///////////////////////////////
// timing_test_measurement.c //
///////////////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <peekpoke.h>

#include "target.h"
#include "timing_test_measurement.h"

unsigned long test_count;
unsigned long error_count;

void reset_test_counts(void)
{
    test_count = 0;
    error_count = 0;
}

void report_test_counts(void)
{
    printf("Tests performed ... : %lu\n", test_count);
    printf("Tests failed ...... : %lu\n", error_count);
    printf("\n");
}

void print_label_value_pair(const char * prefix, const char * label, unsigned long value, unsigned max_label_length)
{   unsigned k;
    printf("%s", prefix);
    k = printf("%s ", label);
    do putchar('.'); while (++k < max_label_length + 7);
    printf(" : %lu\n", value);
}

void print_label_hex_value_pair(const char * prefix, const char * label, unsigned value, unsigned max_label_length)
{   unsigned k;
    printf("%s", prefix);
    k = printf("%s ", label);
    do putchar('.'); while (++k < max_label_length + 7);
    printf(" : 0x%02x\n", value);
}

static void print_test_report(const char * test_description, unsigned test_overhead_cycles, unsigned instruction_cycles, unsigned actual_cycles, LoopSpec loopspec)
{
    (void)loopspec;

    printf("ERROR REPORT FOR \"%s\":\n", test_description);

    print_label_value_pair("  ", "test count"           , test_count           , 20);
    print_label_value_pair("  ", "test overhead cycles" , test_overhead_cycles , 20);
    print_label_value_pair("  ", "instruction cycles"   , instruction_cycles   , 20);
    print_label_value_pair("  ", "actual cycles"        , actual_cycles        , 20);

    printf("END OF ERROR REPORT\n\n");
}


bool run_measurement(const char * test_description, uint8_t * entrypoint, uint8_t flags, LoopSpec loopspec)
{
    extern uint8_t test_overhead_cycles;
    extern uint8_t instruction_cycles;

    unsigned actual_cycles;
    bool success, hook_result;

    actual_cycles = measure_cycles_wrapper(entrypoint);

    ++test_count;

    success = (actual_cycles == test_overhead_cycles + instruction_cycles);

    if (!success)
    {
        ++error_count;
    }

    // If hook_result is false, the hook requests termination.
    hook_result = post_every_measurement_hook(test_description, success, test_count, error_count);

    if (!success)
    {
        print_test_report(test_description, test_overhead_cycles,instruction_cycles, actual_cycles, loopspec);

        if (flags & F_STOP_ON_ERROR)
        {
            return false;
        }
    }

    return hook_result;
}
