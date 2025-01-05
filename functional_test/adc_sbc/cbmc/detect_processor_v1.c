
// This CBMC program aids in finding the shortest possible sequence to reliably classify the variant of
// 6502 processor we're running on.
//
// Precise statement of what we're trying to achieve:
//
//   We want an operation sequence that fills the accumulator with the following value, depending on
//   the CPU type we're running on:
//
// regular 6502                              -> 0
// regular 6502 without decimal mode support -> 1
// regular 65C02                             -> 2

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#include "limited_sim_6502.h"

#define NUMOPS 6

bool always_reaches_target(CpuVariant variant, unsigned operations[NUMOPS], uint8_t target)
{
    // Check if the CPU always (i.e., for all inputs) reaches the target accumulator value,
    // with the Decimal flag disabled on exit.

    CpuState s;
    for (unsigned initial_flag_d = 0; initial_flag_d <= 1; ++initial_flag_d)
    {
        s.FlagD = initial_flag_d;
        for (unsigned initial_flag_c = 0; initial_flag_c <= 1; ++initial_flag_c)
        {
            s.FlagC = initial_flag_c;
            for (unsigned initial_accumulator = 0; initial_accumulator <= 255; initial_accumulator += 255)
            {
                s.Accumulator = initial_accumulator;
                for (unsigned k = 0; k < NUMOPS; ++k)
                {
                    s = operation(variant, s, operations[k]);
                }
                if ((s.Accumulator != target) || (s.FlagD == true))
                {
                    // We found a case that does not satisfy the target and binary-mode-at-exit conditions.
                    return false;
                }
            }
        }
    }
    return true;
}

unsigned nondet_unsigned(void);

unsigned nondet_unsigned_range(unsigned min_value, unsigned max_value)
{
    unsigned result = nondet_unsigned();
    __CPROVER_assume(result >= min_value);
    __CPROVER_assume(result <= max_value);
    return result;
}

// Steps to find the target sequence.
//
// (1) Run with NUMOPS == 6, and with the Accumulator limited to values {0, 255}.
//
//     Runtime: 
//
//     No solutions are found. This means that there is no branchless sequence of 6 operations
//     that can achieve the desired, more general, functionality.
//
// (2) Run with NUMOPS == 7, and with the Accumulator limited to values {0, 255}.
//
//     Runtime: 
//
//     This finds a solution that works at least for these two accumulator values.
//     It looks like this:
//
//
//
//


int main(void)
{
    unsigned operations[NUMOPS];

    // Initialize the operations array with non-deterministic values.
    for (unsigned k = 0; k < NUMOPS; ++k)
    {
        operations[k] = nondet_unsigned_range(0, 0x707);
        //__CPROVER_assume(operations[k] != 0x706); // No RORs.
        //__CPROVER_assume((operations[k] & 0xff00) != 0x0100); // no adds.
    }

    //__CPROVER_assume(operations[0] == 0x701); // SED
    //__CPROVER_assume(operations[1] == 0x00);  // LDA #0
    //__CPROVER_assume(operations[2] == operations[4]); // Same operation.
    //__CPROVER_assume(operations[5] == 0x503); // AND #3
    //__CPROVER_assume(operations[6] == 0x700); // CLD

    uint8_t target_V0 = 0;
    uint8_t target_V1 = 1;
    uint8_t target_V2 = 2;

    bool ok = always_reaches_target(V0, operations, target_V0) &&
              always_reaches_target(V1, operations, target_V1) &&
              always_reaches_target(V2, operations, target_V2);

    // If the "!ok" assertion fails, we found a program that exhibits the desired behavior.
    assert (!ok);
    return 0;
}
