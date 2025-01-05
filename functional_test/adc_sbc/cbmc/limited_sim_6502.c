
////////////////////////
// limited_sim_6502.c //
////////////////////////

#include "limited_sim_6502.h"
#include "6502_adc_sbc.h"

CpuState operation(CpuVariant variant, CpuState s, unsigned op)
{
    if (op < 0x100)
    {
        s.Accumulator = op;
    }
    else if (op < 0x200)
    {
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
        s.Accumulator |= op;
    }
    else if (op < 0x600)
    {
        s.Accumulator &= op;
    }
    else if (op < 0x700)
    {
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
