
//////////////////////////
// timing_test_report.c //
//////////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

//#include "timing_test_report.h"

unsigned long test_count;
unsigned long error_count;

void reset_test_counts(void)
{
    test_count = 0;
    error_count = 0;
}

void print_label_value_pair(const char * prefix, const char * label, unsigned long value, unsigned max_label_length, bool hex_flag)
{   unsigned k;
    printf("%s", prefix);
    k = printf("%s ", label);
    do putchar('.'); while (++k < max_label_length + 7);
    if (hex_flag)
    {
        printf(" : 0x%02lx\n", value);
    }
    else
    {
        printf(" : %lu\n", value);
    }
}

void test_report(const char * test_description, unsigned test_overhead_cycles, unsigned instruction_cycles, unsigned actual_cycles, ...)
{
    ++test_count;

    if (actual_cycles != test_overhead_cycles + instruction_cycles)
    {
        unsigned max_label_length = 20;// Length of "test overhead cycles".
        va_list ap;

        ++error_count;

        printf("ERROR REPORT FOR \"%s\":\n", test_description);

        // The first loop finds the maximum label length.

        va_start(ap, actual_cycles);
        for (;;)
        {
            char * label = va_arg(ap, char *);

            if (label == NULL)
            {
                break;
            }

            if (strlen(label) > max_label_length)
            {
                max_label_length = strlen(label);
            }

            // Skip over the value.
            va_arg(ap, unsigned);
        }
        va_end(ap);

        print_label_value_pair("  ", "test count"           , test_count           , max_label_length, false);
        print_label_value_pair("  ", "error count"          , error_count          , max_label_length, false);
        print_label_value_pair("  ", "test overhead cycles" , test_overhead_cycles , max_label_length, false);
        print_label_value_pair("  ", "instruction cycles"   , instruction_cycles   , max_label_length, false);
        print_label_value_pair("  ", "actual cycles"        , actual_cycles        , max_label_length, false);

        va_start(ap, actual_cycles);
        for (;;)
        {
            char * label;
            unsigned value;

            label = va_arg(ap, char *);

            if (label == NULL)
            {
                break;
            }

            value = va_arg(ap, unsigned);

            print_label_value_pair("  ", label, value, max_label_length, true);
        }
        va_end(ap);

        printf("END OF ERROR REPORT\n\n");
    }
}
