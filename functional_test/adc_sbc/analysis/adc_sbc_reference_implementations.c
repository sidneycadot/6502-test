
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

struct OpResult
{
    uint8_t  Accumulator;
    bool     FlagN;
    bool     FlagV;
    bool     FlagZ;
    bool     FlagC;
};

struct OpResult read_op_result(FILE * fi)
{
    struct {
        uint8_t A;
        uint8_t P;
    } AP;

    size_t fread_result = fread(&AP, 1, sizeof(AP), fi);
    assert(fread_result == sizeof(AP));

    struct OpResult result;

    result.Accumulator = AP.A;
    result.FlagN = (AP.P & 0x80) != 0;
    result.FlagV = (AP.P & 0x40) != 0;
    result.FlagZ = (AP.P & 0x02) != 0;
    result.FlagC = (AP.P & 0x01) != 0;

    return result;
}

bool identical_full(struct OpResult * op1, struct OpResult * op2)
{
    return
        (op1->Accumulator == op2->Accumulator) &&
        (op1->FlagN == op2->FlagN) &&
        (op1->FlagV == op2->FlagV) &&
        (op1->FlagZ == op2->FlagZ) &&
        (op1->FlagC == op2->FlagC);
}

bool identical(struct OpResult * op1, struct OpResult * op2)
{
    return
        (op1->Accumulator) == (op2->Accumulator);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                              //
//                                                         6502 versions                                                        //
//                                                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline struct OpResult adc_6502_binary_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    struct OpResult result;

    const bool initial_accumulator_negative = (initial_accumulator & 0x80) != 0;
    const bool operand_negative = (operand & 0x80) != 0;

    result.Accumulator = (initial_carry_flag + initial_accumulator + operand);
    result.FlagN = (result.Accumulator & 0x80) != 0;
    result.FlagV = (initial_accumulator_negative ^ result.FlagN) & (operand_negative ^ result.FlagN);
    result.FlagZ = result.Accumulator == 0;
    result.FlagC = (initial_carry_flag + initial_accumulator + operand) >= 0x100;

    return result;
}


inline struct OpResult adc_6502_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    struct OpResult result;

    // The N, V, and Z flags are identical to the binary-mode result.

    unsigned a_lo = initial_accumulator & 15;
    unsigned a_hi = initial_accumulator >> 4;

    unsigned o_lo = operand & 15;
    unsigned o_hi = operand >> 4;

    unsigned carry = initial_carry_flag;

    unsigned x_lo = (a_lo + o_lo + carry);

    carry = (x_lo >= 10);

    if (carry)
    {
        x_lo = (x_lo - 10) & 15;
    }

    unsigned x_hi = (a_hi + o_hi + carry);

    x_hi &= 15;

    struct OpResult binary_result = adc_6502_binary_mode(initial_carry_flag, initial_accumulator, operand);

    result.Accumulator = (x_hi << 4) | x_lo;
    result.FlagV = binary_result.FlagV;
    result.FlagN = binary_result.FlagN;
    result.FlagZ = binary_result.FlagZ;

    return result;
}

struct OpResult adc_6502(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return decimal_flag ? adc_6502_decimal_mode(initial_carry_flag, initial_accumulator, operand)
                        : adc_6502_binary_mode (initial_carry_flag, initial_accumulator, operand);
}


inline struct OpResult sbc_6502_binary_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return adc_6502_binary_mode(initial_carry_flag, initial_accumulator, operand ^ 0xff);
}

inline struct OpResult sbc_6502_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    struct OpResult result;

    // The N, V, and Z flags are identical to the binary-mode result.

    struct OpResult binary_result = sbc_6502_binary_mode(initial_carry_flag, initial_accumulator, operand);

    result.FlagV = binary_result.FlagV;
    result.FlagN = binary_result.FlagN;
    result.FlagZ = binary_result.FlagZ;

    return result;
}

struct OpResult sbc_6502(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return decimal_flag ? sbc_6502_decimal_mode(initial_carry_flag, initial_accumulator, operand)
                        : sbc_6502_binary_mode (initial_carry_flag, initial_accumulator, operand);
}

////////////////////////////

inline struct OpResult adc_65c02_binary_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return adc_6502_binary_mode(initial_carry_flag, initial_accumulator, operand);
}

inline struct OpResult adc_65c02_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    (void)initial_carry_flag;
    (void)initial_accumulator;
    (void)operand;

    struct OpResult result;

    result.Accumulator = 0;
    result.FlagN = false;
    result.FlagV = false;
    result.FlagZ = false;
    result.FlagC = false;

    return result;
}

struct OpResult adc_65c02(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return decimal_flag ? adc_65c02_decimal_mode(initial_carry_flag, initial_accumulator, operand)
                        : adc_65c02_binary_mode (initial_carry_flag, initial_accumulator, operand);
}


inline struct OpResult sbc_65c02_binary_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return sbc_6502_binary_mode(initial_carry_flag, initial_accumulator, operand);
}

inline struct OpResult sbc_65c02_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    (void)initial_carry_flag;
    (void)initial_accumulator;
    (void)operand;

    struct OpResult result;

    result.Accumulator = 0;
    result.FlagN = false;
    result.FlagV = false;
    result.FlagZ = false;
    result.FlagC = false;

    return result;
}

struct OpResult sbc_65c02(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return decimal_flag ? sbc_65c02_decimal_mode(initial_carry_flag, initial_accumulator, operand)
                        : sbc_65c02_binary_mode (initial_carry_flag, initial_accumulator, operand);
}

////////////////////////////

typedef struct OpResult testfunc(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand);

void run_tests_on_file(const char * filename, testfunc adc, testfunc sbc, unsigned * count_tests, unsigned * count_success)
{
    FILE * fi = fopen(filename, "rb");
    assert(fi != NULL);

    for (unsigned decimal_flag = 0; decimal_flag <= 1; ++decimal_flag)
    {
        for (unsigned initial_carry_flag = 0; initial_carry_flag <= 1; ++initial_carry_flag)
        {
            for (unsigned initial_accumulator = 0; initial_accumulator <= 255; ++initial_accumulator)
            {
                for (unsigned operand = 0; operand <= 255; ++operand)
                {
                    struct OpResult adc_reference = read_op_result(fi);

                    bool report = (decimal_flag == 1 && initial_carry_flag == 0 && initial_accumulator == 0x00 && operand == 0x9a);

                    if (report)
                    {
                        printf("REF: Accumulator = 0x%02x\n", adc_reference.Accumulator);
                        printf("REF: FlagN = %u\n", adc_reference.FlagN);
                        printf("REF: FlagV = %u\n", adc_reference.FlagV);
                        printf("REF: FlagZ = %u\n", adc_reference.FlagZ);
                        printf("REF: FlagC = %u\n", adc_reference.FlagC);
                    }

                    if (true) // Test ADC instruction.
                    {
                        struct OpResult adc_simulator = adc(decimal_flag, initial_carry_flag, initial_accumulator, operand);

                        if (report)
                        {
                            printf("SIM: Accumulator = 0x%02x\n", adc_simulator.Accumulator);
                            printf("SIM: FlagN = %u\n", adc_simulator.FlagN);
                            printf("SIM: FlagV = %u\n", adc_simulator.FlagV);
                            printf("SIM: FlagZ = %u\n", adc_simulator.FlagZ);
                            printf("SIM: FlagC = %u\n", adc_simulator.FlagC);
                        }

                        *count_tests += 1;
                        if (identical(&adc_simulator, &adc_reference))
                        {
                            *count_success += 1;
                        }
                        else
                        {
                            printf("BAD: decimal_flag %u initial_carry_flag %u && initial_accumulator 0x%02x operand 0x%02x\n",
                                   decimal_flag, initial_carry_flag, initial_accumulator, operand);
                            exit(1);
                        }
                    }

                    struct OpResult sbc_reference = read_op_result(fi);

                    if (false) // Test SBC instruction.
                    {
                        struct OpResult sbc_simulator = sbc(decimal_flag, initial_carry_flag, initial_accumulator, operand);

                        *count_tests += 1;
                        if (identical(&sbc_simulator, &sbc_reference))
                        {
                            *count_success += 1;
                        }
                    }
                }
            }
        }
    }

    fclose(fi);
}

int main(void)
{
    unsigned count_tests = 0;
    unsigned count_success = 0;

    run_tests_on_file("adc_sbc_6502.dat", adc_6502, sbc_6502, &count_tests, &count_success);
    //run_tests_on_file("adc_sbc_65c02.dat", adc_65c02, sbc_65c02, &count_tests, &count_success);

    printf("tests ........ : %u\n", count_tests);
    printf("success ...... : %u\n", count_success);

    return 0;
}
