
////////////////////////
// test_adc_and_sbc.c //
////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "adc_and_sbc.h"

typedef struct OpResult testfunc(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand);

static void read_op_result(FILE * fi, struct OpResult * op_result)
{
    // The test case result are stored in the file as two
    // bytes.
    // The first one is the content of the Accumulator register (A) after the ADB/SBC operation.
    // The second one is the content of the Status register (P) after the ADC/SBC operation.

    struct TestResult {
        uint8_t A;
        uint8_t P;
    } test_result;

    size_t fread_result = fread(&test_result, 1, sizeof(test_result), fi);
    assert(fread_result == sizeof(test_result));

    op_result->Accumulator = test_result.A;
    op_result->FlagN = (test_result.P & 0x80) != 0;
    op_result->FlagV = (test_result.P & 0x40) != 0;
    op_result->FlagZ = (test_result.P & 0x02) != 0;
    op_result->FlagC = (test_result.P & 0x01) != 0;
}

struct ReferenceDataEntry
{
    struct OpResult adc;
    struct OpResult sbc;
};

typedef struct ReferenceDataEntry ReferenceDataArray[2][2][256][256];

static void read_reference_data(const char * filename, ReferenceDataArray reference_data)
{
    FILE * fi = fopen(filename, "rb");
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

static bool identical(struct OpResult * op1, struct OpResult * op2)
{
    return
        (op1->Accumulator == op2->Accumulator) &&
        (op1->FlagN == op2->FlagN) &&
        (op1->FlagV == op2->FlagV) &&
        (op1->FlagZ == op2->FlagZ) &&
        (op1->FlagC == op2->FlagC);
}

static void run_tests_on_file(const char * filename, testfunc adc, testfunc sbc, unsigned * count_tests, unsigned * count_errors)
{
    ReferenceDataArray reference_data;
    read_reference_data(filename, reference_data);

    for (unsigned decimal_flag = 0; decimal_flag <= 1; ++decimal_flag)
    {
        for (unsigned initial_carry_flag = 0; initial_carry_flag <= 1; ++initial_carry_flag)
        {
            for (unsigned initial_accumulator = 0; initial_accumulator <= 255; ++initial_accumulator)
            {
                for (unsigned operand = 0; operand <= 255; ++operand)
                {
                    struct OpResult adc_reference = reference_data[decimal_flag][initial_carry_flag][initial_accumulator][operand].adc;

                    if (true) // Test ADC instruction.
                    {
                        struct OpResult adc_simulator = adc(decimal_flag, initial_carry_flag, initial_accumulator, operand);

                        *count_tests += 1;
                        if (!identical(&adc_simulator, &adc_reference))
                        {
                            *count_errors += 1;
                        }
                    }

                    struct OpResult sbc_reference = reference_data[decimal_flag][initial_carry_flag][initial_accumulator][operand].sbc;

                    if (true) // Test SBC instruction.
                    {
                        struct OpResult sbc_simulator = sbc(decimal_flag, initial_carry_flag, initial_accumulator, operand);

                        *count_tests += 1;
                        if (!identical(&sbc_simulator, &sbc_reference))
                        {
                            *count_errors += 1;
                        }
                    }
                }
            }
        }
    }
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
