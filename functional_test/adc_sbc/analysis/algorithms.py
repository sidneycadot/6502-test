#! /usr/bin/env python3

import numpy as np

testcases = np.load("testcases.npy")

def getbit(n, bitnum):
    return (n >> bitnum) & 1

def adc_binary_6502(initial_carry_flag, initial_accumulator, operand):
    """This implementation reproduces the behavior of the 6502 and 65C02 perfectly."""

    initial_accumulator_negative_flag = getbit(initial_accumulator, 7)
    operand_negative_flag = getbit(operand, 7)

    final_accumulator   = (initial_accumulator + operand + initial_carry_flag) % 256
    final_negative_flag = getbit(final_accumulator, 7)
    final_zero_flag     = int(final_accumulator == 0)
    final_carry_flag    = int((initial_carry_flag + initial_accumulator + operand) > 0xff)
    final_overflow_flag = (initial_accumulator_negative_flag ^ final_negative_flag) & (operand_negative_flag ^ final_negative_flag)

    assert isinstance(final_accumulator, int) and 0 <= final_accumulator <= 255
    assert final_negative_flag in (0, 1)
    assert final_zero_flag in (0, 1)
    assert final_carry_flag in (0, 1)
    assert final_overflow_flag in (0, 1)

    return (final_accumulator, final_negative_flag, final_overflow_flag, final_zero_flag, final_carry_flag)

def adc_decimal_6502(initial_carry_flag, initial_accumulator, operand):

        #if (GET_DF ()) {                                        \
        #    unsigned lo;                                        \
        #    int res;                                            \
        #    lo = (old & 0x0F) + (rhs & 0x0F) + GET_CF ();       \
        #    if (lo >= 0x0A) {                                   \
        #        lo = ((lo + 0x06) & 0x0F) + 0x10;               \
        #    }                                                   \
        #    Regs.AC = (old & 0xF0) + (rhs & 0xF0) + lo;         \
        #    res = (signed char)(old & 0xF0) +                   \
        #          (signed char)(rhs & 0xF0) +                   \
        #          (signed char)lo;                              \
        #    TEST_ZF (old + rhs + GET_CF ());                    \
        #    TEST_SF (Regs.AC);                                  \
        #    if (Regs.AC >= 0xA0) {                              \
        #        Regs.AC += 0x60;                                \
        #    }                                                   \
        #    TEST_CF (Regs.AC);                                  \
        #    SET_OF ((res < -128) || (res > 127));               \
        #    if (CPU == CPU_65C02) {                             \
        #        ++Cycles;                                       \
        #    }                                                   \
        #} else {                                                \

    #return adc_binary_6502(initial_carry_flag, initial_accumulator, operand)

    final_accumulator   = None
    final_negative_flag = None
    final_overflow_flag = None
    final_zero_flag     = None
    final_carry_flag    = None
    return (final_accumulator, final_negative_flag, final_overflow_flag, final_zero_flag, final_carry_flag)

def adc_binary_65c02(initial_carry_flag, initial_accumulator, operand):
    """The binary ADC operation is identical for the 6502 and the 65C02."""
    return adc_binary_6502(initial_carry_flag, initial_accumulator, operand)

def adc_decimal_65c02(initial_carry_flag, initial_accumulator, operand):
    final_accumulator   = None
    final_negative_flag = None
    final_overflow_flag = None
    final_zero_flag     = None
    final_carry_flag    = None
    return (final_accumulator, final_negative_flag, final_overflow_flag, final_zero_flag, final_carry_flag)

def sbc_binary_6502(initial_carry_flag, initial_accumulator, operand):
    """This implementation reproduces the behavior of the 6502 and 65C02 perfectly.
    The binary SBC operation is identical to the ADC operation with the operand bitwise inverted.
    """
    return adc_binary_6502(initial_carry_flag, initial_accumulator, operand ^ 0xff)

def sbc_decimal_6502(initial_carry_flag, initial_accumulator, operand):
    final_accumulator   = None
    final_negative_flag = None
    final_overflow_flag = None
    final_zero_flag     = None
    final_carry_flag    = None
    return (final_accumulator, final_negative_flag, final_overflow_flag, final_zero_flag, final_carry_flag)

def sbc_binary_65c02(initial_carry_flag, initial_accumulator, operand):
    """The binary SBC operation is identical for the 6502 and the 65C02."""
    return sbc_binary_6502(initial_carry_flag, initial_accumulator, operand)

def sbc_decimal_65c02(initial_carry_flag, initial_accumulator, operand):
    """The binary ADC operation is identical for the 6502 and the 65C02."""
    final_accumulator   = None
    final_negative_flag = None
    final_overflow_flag = None
    final_zero_flag     = None
    final_carry_flag    = None
    return (final_accumulator, final_negative_flag, final_overflow_flag, final_zero_flag, final_carry_flag)

function_map = {
    (1, +1, 0): adc_binary_6502,
    (1, +1, 1): adc_decimal_6502,
    (2, +1, 0): adc_binary_65c02,
    (2, +1, 1): adc_decimal_65c02,
    (1, -1, 0): sbc_binary_6502,
    (1, -1, 1): sbc_decimal_6502,
    (2, -1, 0): sbc_binary_65c02,
    (2, -1, 1): sbc_decimal_65c02
}

count_good = 0
count = 0
for testcase in testcases:

    (processor, operation, decimal_flag, initial_carry_flag, initial_accumulator, operand, final_accumulator, final_negative_flag, final_overflow_flag, final_zero_flag, final_carry_flag) = tuple(map(int, testcase))

    if (processor, operation, decimal_flag) != (1, +1, 1):
        continue
    #if not(decimal_flag == 1):
    #    continue

    func = function_map[(processor, operation, decimal_flag)]

    (calc_accumulator, calc_negative_flag, calc_overflow_flag, calc_zero_flag, calc_carry_flag) = func(initial_carry_flag, initial_accumulator, operand)

    ok = (calc_accumulator, calc_negative_flag, calc_overflow_flag, calc_zero_flag, calc_carry_flag) == (final_accumulator, final_negative_flag, final_overflow_flag, final_zero_flag, final_carry_flag)
    #ok = (calc_carry_flag == final_carry_flag)

    if not ok:
        print((processor, operation, decimal_flag), "C", initial_carry_flag, "A", initial_accumulator, "OP", operand, "| A", final_accumulator, final_negative_flag, final_overflow_flag, final_zero_flag, final_carry_flag)
        print((calc_accumulator, calc_negative_flag, calc_overflow_flag, calc_zero_flag, calc_carry_flag))
        break

    count += 1
    count_good += ok

print("count", count, "count_good", count_good)
