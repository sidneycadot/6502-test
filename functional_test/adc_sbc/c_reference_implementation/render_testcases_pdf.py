#! /usr/bin/env python3

import itertools

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

def render_testcases_to_multipage_pdf(testcases, pdf):

    operations = (
        ("ADC/binary" , +1, 0),
        ("SBC/binary" , -1, 0),
        ("ADC/decimal", +1, 1),
        ("SBC/decimal", -1, 1),
    )

    initial_carry_flag_values = (0, 1)

    for initial_carry_flag, (operation_name, operation, decimal_flag)  in itertools.product(initial_carry_flag_values, operations):

        plt.figure(figsize = (16, 8))

        plt.suptitle("{}, initial C={}\nhorizontal axis: initial accumulator, vertical axis: operand".format(operation_name, initial_carry_flag))

        for processor in (1, 2):

            selection = (testcases["processor"] == processor) & (testcases["operation"] == operation) & (testcases["decimal_flag"] == decimal_flag)  & (testcases["initial_carry_flag"] == initial_carry_flag)
            tc = testcases[selection]

            plt.subplot(2, 5, 1 + (processor - 1) * 5)
            plt.imshow(tc["final_accumulator"].reshape(256, 256).transpose(), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            plt.ylabel({1: "6502", 2: "65C02"}[processor])
            if processor == 1:
                plt.title("accumulator (A) register")

            plt.subplot(2, 5, 2 + (processor - 1) * 5)
            plt.imshow(tc["final_negative_flag"].reshape(256, 256).transpose(), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            if processor == 1:
                plt.title("negative (N) flag")

            plt.subplot(2, 5, 3 + (processor - 1) * 5)
            plt.imshow(tc["final_overflow_flag"].reshape(256, 256).transpose(), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            if processor == 1:
                plt.title("overflow (V) flag")

            plt.subplot(2, 5, 4 + (processor - 1) * 5)
            plt.imshow(tc["final_zero_flag"].reshape(256, 256).transpose(), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            if processor == 1:
                plt.title("zero (Z) flag")

            plt.subplot(2, 5, 5 + (processor - 1) * 5)
            plt.imshow(tc["final_carry_flag"].reshape(256, 256).transpose(), origin='lower', extent=(-0.5, 255.5, -0.5, 255.5))
            if processor == 1:
                plt.title("carry (C) flag")

        pdf.savefig()
        plt.close()

def main():

    testcases = np.load("testcases.npy")

    filename = "testcases.pdf"
    print(f"Rendering {filename!r} ...")
    with PdfPages(filename) as pdf:
        render_testcases_to_multipage_pdf(testcases, pdf)

if __name__ == "__main__":
    main()

