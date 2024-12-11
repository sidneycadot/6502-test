
/////////////////////////
// test_6502_adc_sbc.c //
/////////////////////////

// This program reads reference data from files that describes the behavior of the ADC and SBC instructions for both
// the 6502 and the 65C02 processors, and verifies that the implementations provided by the "adc_and_sbc.c" code
// precisely reproduce the behavior recorded in those files.
//
// The original reference files should be produced on hardware (not an emulator) to ensure that they precisely record
// the behavior of the ADC and SBC instructions of the processor.
//
// Such reference files were recorded on an Atari 800 XL (with a "SALLY" 6502 processor) and a Neo6502 development
// board (with a WDC 65C02 processor). The file MD5SUM in this repository contains a hash of those "gold standard"
// files.

#include <stdio.h>
#include <assert.h>

#include "6502_adc_sbc.h"

typedef AddSubResult testfunc(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand);

static void read_op_result(FILE * fi, AddSubResult * result)
{
    // The test case result are stored in the file as two bytes.
    // The first one is the content of the Accumulator register (A) after the ADB/SBC operation.
    // The second one is the content of the Status register (P) after the ADC/SBC operation.

    struct {
        uint8_t A;
        uint8_t P;
    } test_result;

    size_t fread_result = fread(&test_result, 1, sizeof(test_result), fi);
    assert(fread_result == sizeof(test_result));

    result->Accumulator = test_result.A;
    result->FlagN = (test_result.P & 0x80) != 0;
    result->FlagV = (test_result.P & 0x40) != 0;
    result->FlagZ = (test_result.P & 0x02) != 0;
    result->FlagC = (test_result.P & 0x01) != 0;
}

typedef struct {
    AddSubResult adc;
    AddSubResult sbc;
} ReferenceDataEntry;

typedef ReferenceDataEntry ReferenceDataArray[2][2][256][256];

static void read_reference_data(const char * filename, ReferenceDataArray reference_data)
{
    FILE * fi = fopen(filename, "rb");
    assert(fi != NULL);
    for (unsigned decimal_flag = 0; decimal_flag <= 1; ++decimal_flag)
    {
        for (unsigned initial_carry_flag = 0; initial_carry_flag <= 1; ++initial_carry_flag)
        {
            for (unsigned initial_accumulator = 0; initial_accumulator <= 0xff; ++initial_accumulator)
            {
                for (unsigned operand = 0; operand <= 0xff; ++operand)
                {
                    read_op_result(fi, &reference_data[decimal_flag][initial_carry_flag][initial_accumulator][operand].adc);
                    read_op_result(fi, &reference_data[decimal_flag][initial_carry_flag][initial_accumulator][operand].sbc);
                }
            }
        }
    }
    fclose(fi);
}

static bool identical(AddSubResult * r1, AddSubResult * r2)
{
    return
        (r1->Accumulator == r2->Accumulator) &&
        (r1->FlagN == r2->FlagN) &&
        (r1->FlagV == r2->FlagV) &&
        (r1->FlagZ == r2->FlagZ) &&
        (r1->FlagC == r2->FlagC);
}

static void run_tests_on_file(const char * filename, testfunc adc, testfunc sbc, unsigned * count_tests, unsigned * count_errors)
{
    ReferenceDataArray reference_data;

    printf("Running ADC/SBC behavior tests against hardware behavior reference file: %s ...\n", filename);

    read_reference_data(filename, reference_data);

    for (unsigned decimal_flag = 0; decimal_flag <= 1; ++decimal_flag)
    {
        for (unsigned initial_carry_flag = 0; initial_carry_flag <= 1; ++initial_carry_flag)
        {
            for (unsigned initial_accumulator = 0; initial_accumulator <= 255; ++initial_accumulator)
            {
                for (unsigned operand = 0; operand <= 255; ++operand)
                {
                    AddSubResult adc_reference = reference_data[decimal_flag][initial_carry_flag][initial_accumulator][operand].adc;

                    if (true) // Test ADC instruction.
                    {
                        AddSubResult adc_simulator = adc(decimal_flag, initial_carry_flag, initial_accumulator, operand);

                        *count_tests += 1;
                        if (!identical(&adc_simulator, &adc_reference))
                        {
                            *count_errors += 1;
                        }
                    }

                    AddSubResult sbc_reference = reference_data[decimal_flag][initial_carry_flag][initial_accumulator][operand].sbc;

                    if (true) // Test SBC instruction.
                    {
                        AddSubResult sbc_simulator = sbc(decimal_flag, initial_carry_flag, initial_accumulator, operand);

                        *count_tests += 1;
                        if (!identical(&sbc_simulator, &sbc_reference))
                        {
                            *count_errors += 1;
                        }
                    }

                } // end of operand loop
            } // end of initial_accumulator loop
        } // end of initial_carry_flag loop
    } // end of decimal_flag loop
}

int main(void)
{
    unsigned count_tests = 0;
    unsigned count_errors = 0;

    run_tests_on_file("adc_sbc_6502.dat", adc_6502, sbc_6502, &count_tests, &count_errors);
    run_tests_on_file("adc_sbc_65c02.dat", adc_65c02, sbc_65c02, &count_tests, &count_errors);

    printf("tests ....... : %u\n", count_tests);
    printf("errors ...... : %u\n", count_errors);

    return 0;
}
