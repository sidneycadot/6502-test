#! /usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

testcases = np.load("testcases.npy")

print(testcases.shape, testcases.dtype)

operations = (
    ("ADC/binary" , +1, 0),
    ("SBC/binary" , -1, 0),
    ("ADC/decimal", +1, 1),
    ("SBC/decimal", -1, 1),
)

for initial_carry_flag in (0, 1):

    for (operation_name, operation, decimal_flag) in operations:

        plt.clf()
        plt.gcf().set_size_inches(16,8)
        plt.suptitle("{}, initial C={}".format(operation_name, initial_carry_flag))

        for processor in (1, 2):

            selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)  & (testcases["initial_carry_flag"] == initial_carry_flag)
            tc = testcases[selection]
            assert len(tc) == 65536

            plt.subplot(2, 5, 1 + (processor - 1) * 5)
            plt.imshow(tc["final_accumulator"].reshape(256, 256), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            plt.ylabel({1: "6502", 2: "65C02"}[processor])
            if processor == 1:
                plt.title("accumulator (A) register")

            plt.subplot(2, 5, 2 + (processor - 1) * 5)
            plt.imshow(tc["final_negative_flag"].reshape(256, 256), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            if processor == 1:
                plt.title("negative (N) flag")

            plt.subplot(2, 5, 3 + (processor - 1) * 5)
            plt.imshow(tc["final_overflow_flag"].reshape(256, 256), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            if processor == 1:
                plt.title("overflow (V) flag")

            plt.subplot(2, 5, 4 + (processor - 1) * 5)
            plt.imshow(tc["final_zero_flag"].reshape(256, 256), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            if processor == 1:
                plt.title("zero (Z) flag")

            plt.subplot(2, 5, 5 + (processor - 1) * 5)
            plt.imshow(tc["final_carry_flag"].reshape(256, 256), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            if processor == 1:
                plt.title("carry (C) flag")

        plt.show()
