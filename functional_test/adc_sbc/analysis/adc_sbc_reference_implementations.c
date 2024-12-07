
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

void read_op_result(FILE * fi, struct OpResult * op_result)
{
    struct {
        uint8_t A;
        uint8_t P;
    } AP;

    size_t fread_result = fread(&AP, 1, sizeof(AP), fi);
    assert(fread_result == sizeof(AP));

    op_result->Accumulator = AP.A;
    op_result->FlagN = (AP.P & 0x80) != 0;
    op_result->FlagV = (AP.P & 0x40) != 0;
    op_result->FlagZ = (AP.P & 0x02) != 0;
    op_result->FlagC = (AP.P & 0x01) != 0;
}

struct ReferenceDataEntry
{
    struct OpResult adc;
    struct OpResult sbc;
};

typedef struct ReferenceDataEntry ReferenceDataArray[2][2][256][256];

void read_reference_data(const char * filename, ReferenceDataArray reference_data)
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


bool identical(struct OpResult * op1, struct OpResult * op2)
{
    return
        (op1->Accumulator == op2->Accumulator) &&
        (op1->FlagN == op2->FlagN) &&
        (op1->FlagV == op2->FlagV) &&
        (op1->FlagZ == op2->FlagZ) &&
        (op1->FlagC == op2->FlagC);
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

    const bool initial_accumulator_negative = (initial_accumulator & 0x80) != 0;
    const bool operand_negative = (operand & 0x80) != 0;

    // For the 6502 ADC instruction in decimal mode, the Z flag behaves as if we're in binary mode.

    const uint8_t binary_result = (initial_carry_flag + initial_accumulator + operand);
    result.FlagZ = (binary_result == 0);

    // For the 6502 ADC instruction in decimal mode, the Accumulator and the N, V, and C flags behave differently.

    bool carry = initial_carry_flag;

    uint8_t low_nibble = (initial_accumulator & 15) + (operand & 15) + carry;
    if ((carry = low_nibble > 9))
    {
        low_nibble = (low_nibble - 10) & 15;
    }

    uint8_t high_nibble = (initial_accumulator >> 4) + (operand >> 4) + carry;

    // For ADC, the N and V flags are determined based on the high nibble calculated before carry-correction.
    result.FlagN = (high_nibble & 8) != 0;
    result.FlagV = (initial_accumulator_negative ^ result.FlagN) & (operand_negative ^ result.FlagN);

    if ((carry = high_nibble > 9))
    {
        high_nibble = (high_nibble - 10) & 15;
    }

    result.Accumulator = (high_nibble << 4) | low_nibble;
    result.FlagC = carry;

    return result;
}

struct OpResult adc_6502(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return decimal_flag ? adc_6502_decimal_mode(initial_carry_flag, initial_accumulator, operand)
                        : adc_6502_binary_mode (initial_carry_flag, initial_accumulator, operand);
}

inline struct OpResult sbc_6502_binary_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    // SBC in binary mode is identical to the ADC in binary mode with the operand inverted.

    struct OpResult result;

    const bool initial_accumulator_negative = (initial_accumulator & 0x80) != 0;
    const bool operand_nonnegative = (operand & 0x80) == 0;

    const bool borrow = !initial_carry_flag;

    result.Accumulator = initial_accumulator  - operand - borrow;
    result.FlagN = (result.Accumulator & 0x80) != 0;
    result.FlagV = (initial_accumulator_negative ^ result.FlagN) & (operand_nonnegative ^ result.FlagN);
    result.FlagZ = result.Accumulator == 0;
    result.FlagC = initial_accumulator >= operand + borrow;

    return result;
}

inline struct OpResult sbc_6502_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    struct OpResult result;

    // For the 6502 SBC instruction in decimal mode, the N, V, and Z flags behave as if we're in binary mode.

    const bool initial_accumulator_negative = (initial_accumulator & 0x80) != 0;
    const bool operand_nonnegative = (operand & 0x80) == 0;

    bool borrow = !initial_carry_flag;

    const uint8_t binary_result = initial_accumulator  - operand - borrow;
    result.FlagN = (binary_result & 0x80) != 0;
    result.FlagV = (initial_accumulator_negative ^ result.FlagN) & (operand_nonnegative ^ result.FlagN);
    result.FlagZ = (binary_result == 0);

    // For the 6502 SBC instruction in decimal mode, the Accumulator and the C flag behave differently.

    uint8_t low_nibble = (initial_accumulator & 15) - (operand & 15) - borrow;
    if ((borrow = ((low_nibble & 0x80) != 0)))
    {
        low_nibble = (low_nibble + 10) & 15;
    }

    uint8_t high_nibble = (initial_accumulator >> 4) - (operand >> 4) - borrow;
    if ((borrow = ((high_nibble & 0x80) != 0)))
    {
        high_nibble = (high_nibble + 10) & 15;
    }

    result.Accumulator = (high_nibble << 4) | low_nibble;
    result.FlagC = !borrow;

    return result;
}

struct OpResult sbc_6502(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return decimal_flag ? sbc_6502_decimal_mode(initial_carry_flag, initial_accumulator, operand)
                        : sbc_6502_binary_mode (initial_carry_flag, initial_accumulator, operand);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                              //
//                                                         65C02 versions                                                       //
//                                                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline struct OpResult adc_65c02_binary_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    // The behavior in binary mode is identical to the behavior of the 6502.
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
    // The behavior in binary mode is identical to the behavior of the 6502.
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

                    bool report = (decimal_flag == 1 && initial_carry_flag == 0 && initial_accumulator == 0x00 && operand == 0x7a);

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

                    struct OpResult sbc_reference = reference_data[decimal_flag][initial_carry_flag][initial_accumulator][operand].sbc;

                    if (true) // Test SBC instruction.
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
