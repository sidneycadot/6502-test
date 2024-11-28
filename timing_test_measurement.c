
///////////////////////////////
// timing_test_measurement.c //
///////////////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
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

static void print_test_report(const char * test_description, unsigned test_overhead_cycles, unsigned instruction_cycles, unsigned actual_cycles, va_list pass1)
{
    unsigned max_label_length = 20; // Length of "test overhead cycles".

    va_list pass2;

    va_copy(pass2, pass1);

    printf("ERROR REPORT FOR \"%s\":\n", test_description);

    // The first loop finds the maximum label length.

    for (;;)
    {
        char * label = va_arg(pass1, char *);

        if (label == NULL)
        {
            break;
        }

        if (strlen(label) > max_label_length)
        {
            max_label_length = strlen(label);
        }

        // Skip over the value.
        va_arg(pass1, unsigned);
    }

    print_label_value_pair("  ", "test count"           , test_count           , max_label_length);
    print_label_value_pair("  ", "test overhead cycles" , test_overhead_cycles , max_label_length);
    print_label_value_pair("  ", "instruction cycles"   , instruction_cycles   , max_label_length);
    print_label_value_pair("  ", "actual cycles"        , actual_cycles        , max_label_length);

    for (;;)
    {
        char * label;
        unsigned value;

        label = va_arg(pass2, char *);

        if (label == NULL)
        {
            break;
        }

        value = va_arg(pass2, unsigned);

        print_label_hex_value_pair("  ", label, value, max_label_length);
    }

    va_end(pass2);

    printf("END OF ERROR REPORT\n\n");
}


bool run_measurement(const char * test_description, unsigned test_overhead_cycles, unsigned instruction_cycles, uint8_t * entrypoint, bool save_zp_flag, ...)
{
    unsigned actual_cycles;
    bool success;

    actual_cycles = save_zp_flag ? measure_cycles_zp_safe(entrypoint) : measure_cycles(entrypoint);
    ++test_count;

    success = (actual_cycles == test_overhead_cycles + instruction_cycles);

    if (!success)
    {
        ++error_count;
    }

    post_measurement_cycles_hook(test_description, success, test_count, error_count);

    if (!success)
    {
        va_list ap;
        // Print an error report.
        va_start(ap, save_zp_flag);
        print_test_report(test_description, test_overhead_cycles,instruction_cycles, actual_cycles, ap);
        va_end(ap);
    }

    return success;
}
