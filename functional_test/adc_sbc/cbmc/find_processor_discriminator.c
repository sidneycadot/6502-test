
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "6502_adc_sbc.h"

#define NUMOPS 2

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

bool all_ok(Variant variant, unsigned operations[NUMOPS], uint8_t target)
{
  CpuState s;
  for (unsigned initial_flag_d = 0; initial_flag_d <= 1; ++initial_flag_d)
    {
      s.FlagD = initial_flag_d;
      for (unsigned initial_flag_c = 0; initial_flag_c <= 1; ++initial_flag_c)
	{
	  s.FlagC = initial_flag_c;
	  for (unsigned initial_accumulator = 0; initial_accumulator <= 255; initial_accumulator += 17)
	    {
	      s.Accumulator = initial_accumulator;
	      for (unsigned k = 0; k < NUMOPS; ++k)
		{
		  s = operation(s, variant, operations[k]);
		}
	      if ((s.Accumulator != target) || (s.FlagD == true))
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
      //__CPROVER_assume(operations[k] != 0x706); // No RORs.
      //__CPROVER_assume((operations[k] & 0xff00) != 0x0100); // no adds.
    }

  //__CPROVER_assume(operations[0] == 0x701); // SED
  //__CPROVER_assume(operations[1] == 0x00);  // LDA #0
  //__CPROVER_assume(operations[2] == operations[4]); // Same operation.
  //__CPROVER_assume(operations[5] == 0x503); // AND #3
  //__CPROVER_assume(operations[6] == 0x700); // CLD

  //uint8_t target_V0 = nondet_uint8();
  //uint8_t target_V1 = nondet_uint8();
  //uint8_t target_V2 = nondet_uint8();

  uint8_t target_V0 = 0;
  //uint8_t target_V1 = 1;
  uint8_t target_V2 = 1;

  //__CPROVER_assume(target_V0 != target_V1);
  //__CPROVER_assume(target_V0 != target_V2);
  //__CPROVER_assume(target_V1 != target_V2);

  //bool ok = all_ok(V0, operations, target_V0) && all_ok(V1, operations, target_V1) && all_ok(V2, operations, target_V2);
  bool ok = all_ok(V0, operations, target_V0) && all_ok(V2, operations, target_V2);

  assert (!ok);
  //printf("%u %u %u\n", v1, v2, v3);
  return 0;
}
