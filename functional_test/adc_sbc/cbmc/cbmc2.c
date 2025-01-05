// This C program, in combination with the CBMC tool, can be used to find sequences of instructions
// that can distinguish between different sub-types of 6502 processors. In particular, it can model
// the behavior of:
//
// V0: a regular 6502 with decimal mode;
// V1: a simplified 6502 that ignores the D (decimal mode) bit, and always operates in binary mode;
// V2: a 65C02 that supports decimal mode, but behaves differently than a regular 6502 for non-BCD values.
//
// Usage:
//
// cbmc --trace cbmc.c 6502_adc_sbc.c
//
// How it works
// ------------
//
// The program models a subset of the state of a 6502 processor (the D flag, the C flag, and the accumulator)
// and the effect of the following operations on its state:
//
// 0x0000 .. 0x00ff      LDA #imm
// 0x0100 .. 0x01ff      ADC #imm
// 0x0200 .. 0x02ff      SBC #imm
// 0x0300 .. 0x03ff      CMP #imm
// 0x0400 .. 0x04ff      ORA #imm
// 0x0500 .. 0x05ff      AND #imm
// 0x0600 .. 0x06ff      EOR #imm
// 0x0700                CLD
// 0x0701                SED
// 0x0702                CLC
// 0x0703                SEC
// 0x0704                LSR A
// 0x0705                ASL A
// 0x0706                ROR A
// 0x0707                ROL A
//
// These operations simulate the behavior of the corresponding 6502 instructions on the state.
// In particular, the behavior of the ADC and SBC operations is modelled correctly depending
// on whether a CPU of subtype V0, V1, or V2 is used.
//
// The program starts by /nondeterministically/ initializing a short array called "operations"
// with a sequence of operations chosen from the set above.
//
// This means that the operations are "free variables" that CBMC can assign to reach a certain
// desired behavior.
//
// These operation can be subjected to constraints. For example, it is easy to specify that the
// first instruction needs to be a SED and the last instruction should be a CLD.
//
// It will then simulate this short sequence of operations for the CPU types we're interested in,
// checking whether the operation sequence will result in the specified "target" value for the accumulator.

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "6502_adc_sbc.h"

#define NUMOPS 6

typedef enum {
    V0, // A plain old 6502.
    V1, // A 6502 that only implements binary mode for its ADC/SBC operations.
    V2  // A 65C02.
} Variant;

typedef struct
{
    bool FlagD;
    bool FlagC;
    uint8_t Accumulator;
} CpuState;

CpuState operation(CpuState s, Variant variant, unsigned op)
{
    if (op < 0x100)
    {
        // LDA #imm
        s.Accumulator = op;
    }
    else if (op < 0x200)
    {
        // ADC #imm
        AddSubResult r;
        if (variant == V0)
            r = adc_6502(s.FlagD, s.FlagC, s.Accumulator, op);
        else if (variant == V1)
            r = adc_6502(false, s.FlagC, s.Accumulator, op);
        else if (variant == V2)
            r = adc_65c02(s.FlagD, s.FlagC, s.Accumulator, op);
        s.FlagC = r.FlagC;
        s.Accumulator = r.Accumulator;
    }
    else if (op < 0x300)
    {
        // SBC #imm
        AddSubResult r;
        if (variant == V0)
            r = sbc_6502(s.FlagD, s.FlagC, s.Accumulator, op);
        else if (variant == V1)
            r = sbc_6502(false, s.FlagC, s.Accumulator, op);
        else if (variant == V2)
            r = sbc_65c02(s.FlagD, s.FlagC, s.Accumulator, op);
        s.FlagC = r.FlagC;
        s.Accumulator = r.Accumulator;
    }
    else if (op < 0x400)
    {
        // CMP #imm
        AddSubResult r;
        if (variant == V0)
            r = sbc_6502(false, s.FlagC, s.Accumulator, op);
        else if (variant == V1)
            r = sbc_6502(false, s.FlagC, s.Accumulator, op);
        else if (variant == V2)
            r = sbc_65c02(false, s.FlagC, s.Accumulator, op);
        s.FlagC = r.FlagC;
    }
    else if (op < 0x500)
    {
        // ORA #imm
        s.Accumulator |= op;
    }
    else if (op < 0x600)
    {
        // AND #imm
        s.Accumulator &= op;
    }
    else if (op < 0x700)
    {
        // EOR #imm
        s.Accumulator ^= op;
    }
    else if (op == 0x700)
    {
        // CLD
        s.FlagD = false;
    }
    else if (op == 0x701)
    {
        // SED
        s.FlagD = true;
    }
    else if (op == 0x702)
    {
        // CLC
        s.FlagC = false;
    }
    else if (op == 0x703)
    {
        //SEC
        s.FlagC = true;
    }
    else if (op == 0x704)
    {
        // LSR
        s.FlagC = (s.Accumulator & 1) != 0;
        s.Accumulator >>= 1;
    }
    else if (op == 0x705)
    {
        // ASL
        s.FlagC = (s.Accumulator & 0x80) != 0;
        s.Accumulator <<= 1;
    }
    else if (op == 0x706)
    {
        // ROR
        bool f = s.FlagC;
        s.FlagC = (s.Accumulator & 1) != 0;
        s.Accumulator >>= 1;
        if (f) s.Accumulator |= 0x80;
    }
    else if (op == 0x707)
    {
        // ROL
        bool f = s.FlagC;
        s.FlagC = (s.Accumulator & 0x80) != 0;
        s.Accumulator <<= 1;
        if (f) s.Accumulator |= 0x01;
    }
    return s;
}

bool all_ok(Variant variant, unsigned operations[NUMOPS])
{
    const char * hex = "0123456789ABCDEF";
    CpuState s;
    for (unsigned initial_flag_d = 0; initial_flag_d <= 0; ++initial_flag_d)
    {
        s.FlagD = initial_flag_d;
        for (unsigned initial_flag_c = 0; initial_flag_c <= 1; ++initial_flag_c)
        {
            s.FlagC = initial_flag_c;
            for (unsigned initial_accumulator = 0; initial_accumulator <= 255; initial_accumulator += 16)
	    //for (unsigned initial_accumulator = 0; initial_accumulator <= 15; initial_accumulator += 1)
            {
                s.Accumulator = initial_accumulator;
                for (unsigned k = 0; k < NUMOPS; ++k)
                {
                    s = operation(s, variant, operations[k]);
                }
                if (s.Accumulator != hex[initial_accumulator >> 4] || s.FlagD == true)
		  // if (s.Accumulator != hex[initial_accumulator & 15] || s.FlagD == true)
                {
                    // We found a case that does not reach its target state.
                    return false;
                }
            }
        }
    }
    return true;
}

unsigned nondet_unsigned(void);
unsigned nondet_uint8(void);

int main(void)
{
    unsigned operations[NUMOPS];
  
    for (unsigned k = 0; k < NUMOPS; ++k)
    {
        operations[k] = nondet_unsigned();
        __CPROVER_assume(operations[k] <= 0x707);
        //__CPROVER_assume(operations[k] != 0x701); // No SEDs.
        //__CPROVER_assume(operations[k] != 0x706); // No RORs.
        //__/CPROVER_assume(operations[k] != 0x707); // No ROLs.
        //__CPROVER_assume((operations[k] & 0xff00) != 0x0100); // no SBCs.
    }

    //__CPROVER_assume(operations[0] == 0x701); // SED
    //__CPROVER_assume(operations[1] == 0x000); // LDA #imm
    //__CPROVER_assume(operations[2] == operations[4]); // Same operation.
    //__CPROVER_assume(operations[3] == 0x705); // ASL A.
    //__CPROVER_assume(operations[5] == 0x501); // AND #1
    //__CPROVER_assume(operations[6] == 0x700); // CLD

    //__CPROVER_assume(target_V0 != target_V1);
    //__CPROVER_assume(target_V0 != target_V2);
    //__CPROVER_assume(target_V1 != target_V2);

    bool ok = all_ok(V0, operations);
    //bool ok = all_ok(V0, operations, target_V0) && all_ok(V2, operations, target_V2);

    assert(!ok);
    //printf("%u %u %u\n", v1, v2, v3);
    return 0;
}
