#! /usr/bin/env python3

import numpy as np

testcase_list = []

for (processor, filename) in ((1, 'adc_sbc_6502.dat'), (2, 'adc_sbc_65c02.dat')):
    with open(filename, "rb") as fi:
        for D in (0, 1):
            for INITIAL_C in (0, 1):
                for INITIAL_A in range(256):
                    for OPERAND in range(256):
                        for OPERATION in (+1, -1):
                            FINAL_A = fi.read(1)[0]
                            FINAL_P = fi.read(1)[0]
                            FINAL_N = (FINAL_P >> 7) & 1
                            FINAL_V = (FINAL_P >> 6) & 1
                            FINAL_Z = (FINAL_P >> 1) & 1
                            FINAL_C = (FINAL_P >> 0) & 1

                            testcase = (processor, OPERATION, D, INITIAL_C, INITIAL_A, OPERAND, FINAL_A, FINAL_N, FINAL_V, FINAL_Z, FINAL_C)
                            testcase_list.append(testcase)

                            print(f"processor {processor:1d} operation {OPERATION:+2d} decimal_flag {D:1d} initial_carry_flag {INITIAL_C:1d} initial_accumulator 0x{INITIAL_A:02x} operand 0x{OPERAND:02x} final_accumulator 0x{FINAL_A:02x} final_negative_flag {FINAL_N:1d} final_overflow_flag {FINAL_V:1d} final_zero_flag {FINAL_Z:1d} final_carry_flag {FINAL_C:1d}")

testcase_dtype = np.dtype(
    [
        ('processor', np.uint8),
        ('operation', np.int8),
        ('decimal_flag', np.uint8),
        ('initial_carry_flag', np.uint8),
        ('initial_accumulator', np.uint8),
        ('operand', np.uint8),
        ('final_accumulator', np.uint8),
        ('final_negative_flag', np.uint8),
        ('final_overflow_flag', np.uint8),
        ('final_zero_flag', np.uint8),
        ('final_carry_flag', np.uint8)
    ])

testcases = np.array(testcase_list, dtype=testcase_dtype)

#np.save('testcases.npy', testcases)
