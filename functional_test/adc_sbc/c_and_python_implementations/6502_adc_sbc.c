
///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//                                         6502_adc_sbc.c                                        //
//                                                                                               //
//     Hardware-verified implementations of the 6502 and 65C02 "ADC" and "SBC" instructions.     //
//     Implemented in December 2024 by Sidney Cadot. Use this code in whatever way you like.     //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "6502_adc_sbc.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//                      Binary mode: 6502 and 65C02 versions are identical.                      //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

// These functions replicate the behavior of the ADC and SBC instructions in binary mode.
//
// The behavior of the ADC and SBC instructions in binary mode is clear and well-defined.
// In contrast with the somewhat fuzzy semantics of decimal mode, it is clear what values
// the accumulator and the CPU flags (N/V/Z/C) should take on for all possible inputs.
//
// The 6502 and the 65C02 implement this behavior correctly and identically.
//
// An interesting fact is that the SBC operation behaves precisely like an ADC operation with just
// the operand's bits inverted, and vice versa.

static inline AddSubResult adc_binary_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    AddSubResult result;

    result.Accumulator = initial_carry_flag + initial_accumulator + operand;
    result.FlagN = result.Accumulator >= 0x80;
    result.FlagV = ((initial_accumulator >= 0x80) ^ result.FlagN) & ((operand >= 0x80) ^ result.FlagN);
    result.FlagZ = result.Accumulator == 0;
    result.FlagC = initial_carry_flag + initial_accumulator + operand >= 0x100;

    return result;
}

static inline AddSubResult sbc_binary_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{

    AddSubResult result;

    const bool borrow = !initial_carry_flag;

    result.Accumulator = initial_accumulator  - operand - borrow;
    result.FlagN = result.Accumulator >= 0x80;
    result.FlagV = ((initial_accumulator >= 0x80) ^ result.FlagN) & ((operand < 0x80) ^ result.FlagN);
    result.FlagZ = result.Accumulator == 0;
    result.FlagC = initial_accumulator >= operand + borrow;

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//             Decimal mode: 6502-specific versions of the ADC and SBC instructions.             //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

// These two routines reproduce the ADC and SBC behavior of a hardware 6502, even when the
// accumulator and/or the operand are not valid BCD (binary coded decimal) values.
//
// For both the ADC and SBC instructions, the way the accumulator and the carry (C) flag are used
// and updated is correct in case the accumulator and operand are valid BCD values upon entry.
//
// The behavior of the N, V, and Z flags is weird. The Z flag is determined as if the calculation
// were performed in binary mode. The same is true for the N and V flags in case of the SBC
// instruction. In contrast, for the ADC instruction, the N and V flags are determined based on an
// intermediate value of the high nibble, and their behavior defies simple description.

static inline AddSubResult adc_6502_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    AddSubResult result;

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

    // For the 6502 ADC instruction in decimal mode, the N and V flags are
    // based on the high nibble before carry-correction.

    result.FlagN = (high_nibble & 8) != 0;
    result.FlagV = ((initial_accumulator >= 0x80) ^ result.FlagN) & ((operand >= 0x80) ^ result.FlagN);

    if ((carry = high_nibble > 9))
    {
        high_nibble = (high_nibble - 10) & 15;
    }

    result.Accumulator = (high_nibble << 4) | low_nibble;
    result.FlagC = carry;

    return result;
}

static inline AddSubResult sbc_6502_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    AddSubResult result;

    // For the 6502 SBC instruction in decimal mode, the N, V, and Z flags behave as in binary mode.

    bool borrow = !initial_carry_flag;

    const uint8_t binary_result = initial_accumulator  - operand - borrow;
    result.FlagN = (binary_result >= 0x80);
    result.FlagV = ((initial_accumulator >= 0x80) ^ result.FlagN) & ((operand < 0x80) ^ result.FlagN);
    result.FlagZ = (binary_result == 0);

    // For the 6502 SBC instruction in decimal mode, the Accumulator and the C flag behave differently.

    uint8_t low_nibble = (initial_accumulator & 15) - (operand & 15) - borrow;
    if ((borrow = low_nibble >= 0x80))
    {
        low_nibble = (low_nibble + 10) & 15;
    }

    uint8_t high_nibble = (initial_accumulator >> 4) - (operand >> 4) - borrow;
    if ((borrow = high_nibble >= 0x80))
    {
        high_nibble = (high_nibble + 10) & 15;
    }

    result.Accumulator = (high_nibble << 4) | low_nibble;
    result.FlagC = !borrow;

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//             Decimal mode: 65C02-specific versions of the ADC and SBC instructions.            //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

// These two routines reproduce the ADC and SBC behavior of a hardware 65C02, even when the
// accumulator and/or the operand are not valid BCD (binary coded decimal) values.
//
// For both the ADC and SBC instructions, the way the accumulator and the carry (C) flag are used
// and updated is correct in case the accumulator and operand are valid BCD values upon entry.
//
// The value of the N and Z flags simply corresponds to the value of the accumulator at the end
// of execution of the ADC or SBC instruction: they indicate if the most significant bit of the
// accumulator is set (N flag), and whether the accumulator is zero (Z flag).
//
// The value of the overflow (V) flag is weird, and defies a simple description.

static inline AddSubResult adc_65c02_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    AddSubResult result;

    bool carry = initial_carry_flag;

    uint8_t low_nibble = (initial_accumulator & 15) + (operand  & 15) + carry;
    if ((carry = low_nibble > 9))
    {
        low_nibble -= 10;
    }
    low_nibble &= 15;

    uint8_t high_nibble = (initial_accumulator >> 4) + (operand  >> 4) + carry;

    const bool PrematureFlagN = (high_nibble & 8) != 0;

    if ((carry = high_nibble > 9))
    {
        high_nibble -= 10;
    }
    high_nibble &= 15;

    result.Accumulator = (high_nibble << 4) | low_nibble;
    result.FlagN = (result.Accumulator >= 0x80);
    result.FlagV = ((initial_accumulator >= 0x80) ^ PrematureFlagN) & ((operand >= 0x80) ^ PrematureFlagN);
    result.FlagZ = (result.Accumulator == 0x00);
    result.FlagC = carry;

    return result;
}

static inline AddSubResult sbc_65c02_decimal_mode(const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    AddSubResult result;

    bool borrow = !initial_carry_flag;

    uint8_t low_nibble = (initial_accumulator & 15) - (operand & 15) - borrow;
    if ((borrow = (low_nibble >= 0x80)))
    {
        low_nibble += 10;
    }
    // low_nibble still being negative here, strangely, influences the high nibble.
    const bool low_nibble_still_negative = (low_nibble >= 0x80);
    low_nibble &= 15;

    uint8_t high_nibble = (initial_accumulator >> 4) - (operand >> 4) - borrow;

    const bool PrematureFlagN = (high_nibble & 8) != 0;

    if ((borrow = (high_nibble >= 0x80)))
    {
        high_nibble += 10;
    }
    high_nibble -= low_nibble_still_negative;
    high_nibble &= 15;

    result.Accumulator = (high_nibble << 4) | low_nibble;
    result.FlagN = (result.Accumulator >= 0x80);
    result.FlagV = ((initial_accumulator >= 0x80) ^ PrematureFlagN) & ((operand < 0x80) ^ PrematureFlagN);
    result.FlagZ = (result.Accumulator == 0x00);
    result.FlagC = !borrow;

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                               //
//                Entry points to ADC/SBC implementations for the 6502 and 65C02.                //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

AddSubResult adc_6502(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return (decimal_flag ? adc_6502_decimal_mode : adc_binary_mode)(initial_carry_flag, initial_accumulator, operand);
}

AddSubResult sbc_6502(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return (decimal_flag ? sbc_6502_decimal_mode : sbc_binary_mode)(initial_carry_flag, initial_accumulator, operand);
}

AddSubResult adc_65c02(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return (decimal_flag ? adc_65c02_decimal_mode : adc_binary_mode)(initial_carry_flag, initial_accumulator, operand);
}

AddSubResult sbc_65c02(const bool decimal_flag, const bool initial_carry_flag, const uint8_t initial_accumulator, const uint8_t operand)
{
    return (decimal_flag ? sbc_65c02_decimal_mode : sbc_binary_mode)(initial_carry_flag, initial_accumulator, operand);
}
