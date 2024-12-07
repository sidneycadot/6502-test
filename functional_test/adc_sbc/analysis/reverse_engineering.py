#! /usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

testcases = np.load("testcases.npy")
print(testcases.shape, testcases.dtype)

if True: # ADC in decimal mode on the 6502.

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

if True:  # SBC in decimal mode on the 6502.

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


if False:

    processor = 2
    operation = +1
    decimal_flag = 1
    initial_carry_flag = 0
    selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)  & (testcases["initial_carry_flag"] == initial_carry_flag)
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

    left  = fv
    right = xv

    left  = left.reshape(256, 256)
    right = right.reshape(256, 256)

    plt.subplot(131)
    plt.imshow(left, origin="lower")
    plt.subplot(132)
    plt.imshow(right, origin="lower")
    plt.subplot(133)
    plt.imshow(right - left, origin="lower")

    print("left==right  -->", np.all(left==right))

    plt.show()

    print("xa==fa", np.all(xa == fa))
    print("xc==fc", np.all(xc == fc))
    print("xz==fz", np.all(xz == fz))
    print("xn==fn", np.all(xn == fn))
    print("xv==fv", np.all(xv == fv))




if False:
    # SBC decimal mode 6502, carry is clear: check the behavor of the flags.

    processor = 1
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

    # First do a binary-mode addition, to get the N, Z, V flags.

    binary_result = (ia - op - (1 - ic))

    xz = (binary_result == 0)
    xn = (binary_result >= 0x80)
    xv = ((ia >= 0x80) ^ xn) & ((~op >= 0x80) ^ xn)

    plt.show()

    # Now do the decimal-mode work.

    ialo  = ia % 16
    oplo  = op % 16

    iahi  = ia // 16
    ophi  = op // 16

    carry = ic

    xlo   = ialo + carry + (9 - oplo)
    carry = (xlo - 10) <= 127
    xlo   = xlo - 10 * carry
    xlo   = xlo % 16

    xhi   = iahi + carry + (9 - ophi)
    carry = (xhi - 10) <= 127
    xhi   = xhi - 10 * carry
    xhi   = xhi % 16

    xa    = 16 * xhi + xlo
    xc    = carry

    print("SBC-decimal, 6502:")
    print("  xa==fa", np.all(xa == fa))
    print("  xc==fc", np.all(xc == fc))
    print("  xz==fz", np.all(xz == fz))
    print("  xn==fn", np.all(xn == fn))
    print("  xv==fv", np.all(xv == fv))
    print()

    left  = fv
    right = xv

    print("left==right", np.all(left==right))

    left  = left.reshape(256, 256)
    right = right.reshape(256, 256)

    plt.subplot(131)
    plt.imshow(left, origin="lower")
    plt.subplot(132)
    plt.imshow(right, origin="lower")
    plt.subplot(133)
    plt.imshow(right - left, origin="lower")

    plt.show()

if False:
    # SBC decimal mode 6502, carry is set: check the behavor of the flags.

    processor = 1
    operation = -1
    decimal_flag = 1
    initial_carry_flag = 1
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

    carry = ic

    xlo   = ialo + carry + (9 - oplo)
    carry = (xlo - 10) <= 127
    xlo   = xlo - 10 * carry
    xlo   = xlo % 16

    xhi   = iahi + carry + (9 - ophi)
    carry = (xhi - 10) <= 127
    xhi   = xhi - 10 * carry
    xhi   = xhi % 16

    xa    = 16 * xhi + xlo
    xc    = carry

    print("SBC-decimal, 6502:")
    print("  xa==fa", np.all(xa == fa))
    print("  xc==fc", np.all(xc == fc))
    print("  xz==fz", np.all(xz == fz))
    print("  xn==fn", np.all(xn == fn))
    print("  xv==fv", np.all(xv == fv))
    print()

    left  = fv
    right = xv

    print("left==right", np.all(left==right))

    left  = left.reshape(256, 256)
    right = right.reshape(256, 256)

    plt.subplot(131)
    plt.imshow(left, origin="lower")
    plt.subplot(132)
    plt.imshow(right, origin="lower")
    plt.subplot(133)
    plt.imshow(right - left, origin="lower")

    plt.show()
