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

    raise NotImplementedError()

    processor = 2
    operation = +1
    decimal_flag = 1
    selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)
    tc = testcases[selection]
    assert len(tc) == 65536

    ia = tc["initial_accumulator"]
    ic = tc["initial_carry_flag"]
    op = tc["operand"]
    fa = tc["final_accumulator"]
    fc = tc["final_carry_flag"]
    fz = tc["final_zero_flag"]
    fn = tc["final_negative_flag"]
    fv = tc["final_overflow_flag"]

    ialo = ia % 16
    oplo = op % 16

    iahi = ia // 16
    ophi = op // 16

    carry = ic

    xlo   = ialo + carry + oplo
    carry = xlo > 9
    xlo   = xlo - 10 * carry
    xlo   = xlo % 16

    xhi   = iahi + carry + ophi
    carry = xhi > 9
    xhi   = xhi - 10 * carry
    xhi   = xhi % 16

    xa = 16 * xhi + xlo
    xc = carry
    xz = (xa == 0)
    xn = (xa >= 0x80)

    print("ADC-decimal, 65C02:")
    print("  xa==fa", np.all(xa == fa))
    print("  xc==fc", np.all(xc == fc))
    print("  xz==fz", np.all(xz == fz))
    print("  xn==fn", np.all(xn == fn))
    print("  xv==fv", np.all(xv == fv))
    print()


def figure_out(testcases):

    # SBC decimal mode 6502, carry is set: check the behavor of the flags.

    processor = 2
    operation = -1
    decimal_flag = 1
    initial_carry_flag = 0
    selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)  & (testcases["initial_carry_flag"] == initial_carry_flag)
    tc = testcases[selection]
    assert len(tc) == 65536

    ia = tc["initial_accumulator"]
    ic = tc["initial_carry_flag"]
    op = tc["operand"]
    fa = tc["final_accumulator"]
    fn = tc["final_negative_flag"]
    fv = tc["final_overflow_flag"]
    fz = tc["final_zero_flag"]
    fc = tc["final_carry_flag"]

    ialo  = ia % 16
    oplo  = op % 16

    iahi  = ia // 16
    ophi  = op // 16

    borrow = 1 - ic

    xlo    = ialo - oplo - borrow

    STRANGE = xlo.astype(np.int8) < -10

    borrow = xlo >= 0x80
    xlo    = xlo + 10 * borrow
    xlo    = xlo % 16

    xhi    = iahi - ophi - borrow

    #print("zz", np.unique(zz))
    xn = (xhi & 8) != 0
    xv = ((ia >= 0x80) ^ xn) & (((255-op) >= 0x80) ^ xn)

    borrow = xhi >= 0x80
    xhi    = xhi + 10 * borrow
    xhi   -= STRANGE
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

    left  = fv
    right = xv

    print("MISMATCHES:", np.sum(left != right))
    print("left==right", np.all(left==right))

    left  = left.reshape(256, 256)
    right = right.reshape(256, 256)
    diff = (right-left)

    plt.subplot(131)
    plt.imshow(left, origin="lower")
    plt.subplot(132)
    plt.imshow(right, origin="lower")
    plt.subplot(133)
    plt.imshow(diff, origin="lower")

    plt.show()


def main():
    testcases = np.load("testcases.npy")
    print(testcases.shape, testcases.dtype)
    #test_adc_decimal_mode_6502(testcases)
    #test_sbc_decimal_mode_6502(testcases)
    #test_adc_decimal_mode_65c02(testcases)
    figure_out(testcases)

if __name__ == "__main__":
    main()
