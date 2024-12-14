
///////////////////////////////
// timing_test_measurement.c //
///////////////////////////////

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "target.h"
#include "timing_test_measurement.h"

// Interface from higher-level routines, via global variables.

uint8_t num_zpage_preserve; // How many zero-pages addresses should the test preserve?
uint8_t zpage_preserve[2];  // Zero page addresses to preserve while the test executes (0, 1, or 2 values).

static const char * m_opcode_description;
static ParSpec      m_parspec;

uint8_t par1;
uint8_t par2;
uint8_t par3;
uint8_t par4;

unsigned     m_test_overhead_cycles;
unsigned     m_instruction_cycles;

unsigned long opcode_count;
unsigned long measurement_count;
unsigned long error_count;

void reset_test_counts(void)
{
    opcode_count = 0;
    measurement_count = 0;
    error_count = 0;
}

void report_test_counts(void)
{
    printf("Opcodes tested ........... : %lu\n", opcode_count);
    printf("Measurements performed ... : %lu\n", measurement_count);
    printf("Measurements failed ...... : %lu\n", error_count);
    printf("\n");
}

void prepare_opcode_tests(const char * opcode_description, ParSpec parspec)
{
    m_opcode_description = opcode_description;
    m_parspec = parspec;
    ++opcode_count;
    pre_every_test_hook(opcode_description);
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

static void print_test_report(unsigned actual_cycles)
{
    uint8_t npar;

    printf("ERROR REPORT FOR \"%s\":\n", m_opcode_description);

    print_label_value_pair("  ", "opcode count"         , opcode_count           , 20);
    print_label_value_pair("  ", "test overhead cycles" , m_test_overhead_cycles , 20);
    print_label_value_pair("  ", "instruction cycles"   , m_instruction_cycles   , 20);
    print_label_value_pair("  ", "actual cycles"        , actual_cycles          , 20);

    switch (m_parspec)
    {
        case Par1_OpcodeOffset:
        case Par1_ClockCycleCount:
            npar = 1;
            break;
        case Par12_OpcodeOffset_Immediate:
        case Par12_OpcodeOffset_ZPage:
        case Par12_OpcodeOffset_AbsOffset:
        case Par12_OpcodeOffset_Displacement:
            npar = 2;
            break;
        case Par123_OpcodeOffset_ZPage_XReg:
        case Par123_OpcodeOffset_ZPage_YReg:
        case Par123_OpcodeOffset_AbsOffset_XReg:
        case Par123_OpcodeOffset_AbsOffset_YReg:
        case Par123_OpcodeOffset_BranchDisplacement_TakenNotTaken:
            npar = 3;
            break;
        case Par1234_OpcodeOffset_ZPage_XReg_AbsOffset:
        case Par1234_OpcodeOffset_ZPage_AbsOffset_YReg:
            npar = 4;
            break;
        default:
            assert(false);
            npar = 4;
    }

    if (npar >= 1)
    {
        print_label_hex_value_pair("  ", "par1", par1, 20);
    }
    if (npar >= 2)
    {
        print_label_hex_value_pair("  ", "par2", par2, 20);
    }
    if (npar >= 3)
    {
        print_label_hex_value_pair("  ", "par3", par3, 20);
    }
    if (npar >= 4)
    {
        print_label_hex_value_pair("  ", "par4", par4, 20);
    }

    printf("END OF ERROR REPORT\n\n");
}


bool execute_single_opcode_test(uint8_t * entrypoint, uint8_t flags)
{
    unsigned actual_cycles;
    bool success, hook_result;

    actual_cycles = measure_cycles_wrapper(entrypoint);

    ++measurement_count;

    success = (actual_cycles == m_test_overhead_cycles + m_instruction_cycles);

    if (!success)
    {
        ++error_count;
    }

    // If hook_result is false, the hook requests termination.
    hook_result = post_every_measurement_hook(success, opcode_count, measurement_count, error_count);

    if (!success)
    {
        print_test_report(actual_cycles);

        if (flags & F_STOP_ON_ERROR)
        {
            return false;
        }
    }

    return hook_result;
}
