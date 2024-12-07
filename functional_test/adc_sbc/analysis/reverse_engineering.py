#! /usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

testcases = np.load("testcases.npy")
print(testcases.shape, testcases.dtype)

if False: # ADC in decimal mode on the 6502.

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
    fc = tc["final_carry_flag"]

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
    carry = xhi > 9
    xhi   = xhi - 10 * carry
    xhi   = xhi % 16

    xa    = xhi * 16 + xlo
    xc    = carry

    print("xa==fa", np.all(xa == fa))
    print("xc==fc", np.all(carry == fc))

if False:  # SBC in decimal mode on the 6502.

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
    fc = tc["final_carry_flag"]

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

    left  = fc
    right = xc

    print("xa==fa", np.all(xa == fa))
    print("xc==fc", np.all(xc == fc))


if True:

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

