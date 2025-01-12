
////////////////////////
// limited_sim_6502.h //
////////////////////////

#ifndef LIMITED_SIM_6502_H
#define LIMITED_SIM_6502_H

// This code models a subset of the state of a 6502 processor (the D flag, the C flag, and the accumulator)
// and the effect of the following operations on its state.
//
// It can be used to search for short, branch-free code fragments in the 6502 and its variants that do
// certain interesting things.
//
// The code models a subset of the 6502 registers, as well as a small subset of its instruction set.
//
// The following registers are modeled:
//
// - the A register
// - status bits C (carry) and D (decimal mode enabled)
//
// The following operations are modeled:
//
// operation value       6502 instruction
// ----------------      ----------------
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
// These operations simulate the behavior of the corresponding 6502 instructions on the modeled
// state. In particular, the behavior of the ADC and SBC operations is modelled correctly depending
// on whether a CPU of subtype V0, V1, or V2 is used.

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    V0, // A plain old 6502.
    V1, // A 6502 that only implements binary mode for its ADC/SBC operations. The D bit is ignored.
    V2  // A 65C02.
} CpuVariant;

typedef struct
{
    bool FlagD;
    bool FlagC;
    uint8_t Accumulator;
} CpuState;

CpuState operation(CpuVariant variant, CpuState s, unsigned op);

#endif
