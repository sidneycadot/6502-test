#! /usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt


def test_adc_decimal_mode_6502(testcases):

    processor = 1
    operation = +1
    decimal_flag = 1
    selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)
    tc = testcases[selection]
    assert len(tc) == 131072

    ia = tc["initial_accumulator"]
    ic = tc["initial_carry_flag"]
    op = tc["operand"]
    fa = tc["final_accumulator"]
    fn = tc["final_negative_flag"]
    fv = tc["final_overflow_flag"]
    fz = tc["final_zero_flag"]
    fc = tc["final_carry_flag"]

    # First do a binary-mode addition, to get the Z flag.

    binary_result = (ia + op + ic)
    xz = (binary_result == 0)

    # Now do the decimal-mode work.

    ialo  = ia % 16
    oplo  = op % 16

    iahi  = ia // 16
    ophi  = op // 16

    carry = ic

    xlo   = ialo + carry + oplo
    carry = xlo > 9
    xlo   = xlo - 10 * carry
    xlo   = xlo % 16

    xhi   = iahi + carry + ophi

    xn = (xhi & 8) != 0
    xv = ((ia >= 0x80) ^ xn) & ((op >= 0x80) ^ xn)

    carry = xhi > 9
    xhi   = xhi - 10 * carry
    xhi   = xhi % 16

    xa    = xhi * 16 + xlo
    xc    = carry

    print("ADC-decimal, 6502:")
    print("  xa==fa", np.all(xa == fa))
    print("  xn==fn", np.all(xn == fn))
    print("  xv==fv", np.all(xv == fv))
    print("  xz==fz", np.all(xz == fz))
    print("  xc==fc", np.all(xc == fc))
    print()


def test_sbc_decimal_mode_6502(testcases):

    processor = 1
    operation = -1
    decimal_flag = 1
    selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)
    tc = testcases[selection]
    assert len(tc) == 131072

    ia = tc["initial_accumulator"]
    ic = tc["initial_carry_flag"]
    op = tc["operand"]
    fa = tc["final_accumulator"]
    fn = tc["final_negative_flag"]
    fv = tc["final_overflow_flag"]
    fz = tc["final_zero_flag"]
    fc = tc["final_carry_flag"]

    # First do a binary-mode addition, to get the N, Z, V flags.

    binary_result = (ia - op - (1 - ic))

    xz = (binary_result == 0)
    xn = (binary_result >= 0x80)
    xv = ((ia >= 0x80) ^ xn) & ((~op >= 0x80) ^ xn)

    # Now do the decimal-mode work.

    ialo  = ia % 16
    oplo  = op % 16

    iahi  = ia // 16
    ophi  = op // 16

    borrow = 1 - ic

    xlo    = ialo - oplo - borrow
    borrow = (xlo >= 0x80)
    xlo    = xlo + 10 * borrow
    xlo    = xlo % 16

    xhi    = iahi - ophi - borrow
    borrow = (xhi >= 0x80)
    xhi    = xhi + 10 * borrow
    xhi    = xhi % 16

    xa    = 16 * xhi + xlo

    xc    = 1 - borrow

    print("SBC-decimal, 6502:")
    print("  xa==fa", np.all(xa == fa))
    print("  xc==fc", np.all(xc == fc))
    print("  xz==fz", np.all(xz == fz))
    print("  xn==fn", np.all(xn == fn))
    print("  xv==fv", np.all(xv == fv))
    print()

def test_adc_decimal_mode_65c02(testcases):

    processor = 2
    operation = +1
    decimal_flag = 1
    selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)
    tc = testcases[selection]
    assert len(tc) == 131072

    ia = tc["initial_accumulator"]
    ic = tc["initial_carry_flag"]
    op = tc["operand"]
    fa = tc["final_accumulator"]
    fc = tc["final_carry_flag"]
    fz = tc["final_zero_flag"]
    fn = tc["final_negative_flag"]
    fv = tc["final_overflow_flag"]

    ialo  = ia % 16
    oplo  = op % 16

    iahi  = ia // 16
    ophi  = op // 16

    carry = ic

    xlo   = ialo + carry + oplo
    carry = xlo > 9
    xlo   = xlo - 10 * carry
    xlo   = xlo % 16

    xhi   = iahi + carry + ophi

    xn = (xhi & 8) != 0
    xv = ((ia >= 0x80) ^ xn) & ((op >= 0x80) ^ xn)

    carry = xhi > 9
    xhi   = xhi - 10 * carry
    xhi   = xhi % 16

    xa    = xhi * 16 + xlo
    xc    = carry

    xn = (xa >= 0x80)
    xz = (xa == 0)

    print("ADC-decimal, 65C02:")
    print("  xa==fa", np.all(xa == fa))
    print("  xn==fn", np.all(xn == fn))
    print("  xv==fv", np.all(xv == fv))
    print("  xz==fz", np.all(xz == fz))
    print("  xc==fc", np.all(xc == fc))
    print()


def test_sbc_decimal_mode_65c02(testcases):

    processor = 2
    operation = -1
    decimal_flag = 1
    selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)
    tc = testcases[selection]
    assert len(tc) == 131072

    ia = tc["initial_accumulator"]
    ic = tc["initial_carry_flag"]
    op = tc["operand"]
    fa = tc["final_accumulator"]
    fc = tc["final_carry_flag"]
    fz = tc["final_zero_flag"]
    fn = tc["final_negative_flag"]
    fv = tc["final_overflow_flag"]

    ialo  = ia % 16
    oplo  = op % 16

    iahi  = ia // 16
    ophi  = op // 16

    borrow = 1 - ic

    xlo    = ialo - oplo - borrow

    borrow = (xlo >> 7)
    xlo    = xlo + 10 * borrow
    xlo_negative = (xlo >> 7)   # xlo still being negative, strangely, influences the high nibble.
    xlo    = xlo % 16

    xhi    = iahi - ophi - borrow

    xn = (xhi & 8) != 0
    xv = ((ia >= 0x80) ^ xn) & ((op < 0x80) ^ xn)

    borrow = (xhi >> 7)
    xhi    = xhi + 10 * borrow
    xhi   -= xlo_negative
    xhi    = xhi % 16

    xa    = xhi * 16 + xlo
    xc    = 1 - borrow

    xn = (xa >= 0x80)
    xz = (xa == 0)

    print("SBC-decimal, 65C02:")
    print("  xa==fa", np.all(xa == fa))
    print("  xc==fc", np.all(xc == fc))
    print("  xz==fz", np.all(xz == fz))
    print("  xn==fn", np.all(xn == fn))
    print("  xv==fv", np.all(xv == fv))
    print()


def main():
    testcases = np.load("testcases.npy")
    test_adc_decimal_mode_6502(testcases)
    test_sbc_decimal_mode_6502(testcases)
    test_adc_decimal_mode_65c02(testcases)
    test_sbc_decimal_mode_65c02(testcases)

if __name__ == "__main__":
    main()
