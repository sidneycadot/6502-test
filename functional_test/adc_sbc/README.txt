The ADC and SBC instructions in the 6502 and 65C02 processors
=============================================================

This directory contains tools and analysis of the most complex instructions in the arsenal
of the 6502 and 65C02: the ADC (add-with-carry) and SBC (subtract-with-carry) instructions.

There's a bunch of things that make ADC and SBC complicated for emulators to get fully right;
and there's a bunch of behaviors that are inherently complex.

Binary Mode and Decimal Mode
----------------------------

The 6502 has a "Decimal Flag" which affects the operation of the ADC and SBC instructions.
When the D flag is 0, ADC and SBC operate in "binary mode"; when the D flag is 1, they
operate in "decimal mode". This flag can be set to 1 with the "SED" instruction, and set
to 0 with the "CLD" instruction.

The ADC and SBC instructions are quite well-behaved and understandable in binary mode.
That's the easy part, although the interpretation of the "overflow" flag is something
that many people struggle with. We will discuss this below.

Decimal mode, on the other hand, is somewhat of a bolt-on.

It tries to support addition and subtraction in the situation where the accumulator
and its ADC/SBC operand bytes represent BCD (Binary Coded Decimal) numbers.
For BCD numbers, the high and low nibble values of a byte should only have the values
0..9, and the number represented by the byte is 0 to 99.

This immediately means that only 100 out of the possible 256 bit patterns in the
accumulator and operand bytes are meaningful. ADC and SBC are intended to be used
with those 100 values, and during design, the behavior of these instructions when
this candition is violated was not given consideration.

However, the behavior when non-BCD numbers are fed to the ADC and SBC instruction
while in Decimal Mode is fully deterministic, and the behavior can be observed on
a real-life 6502 or 65C02 processor.

It's just that the behavior is different on the 6502 and 65C02; and the behavior
is especially weird for the status flags. In the 6502, not much attention was
given to the behavior of the Z, N, and V flags of the processor after an ADC
or SBC operation. The behavior of the carry flag (at least when inputs are used
that are proper BCD numbers) is implemented to make it possible to do BCD
calculations where numbers span multiple bytes.

On the 65C02, the behavior of the Z and N flags is altered to support the BCD-
equivalents of the notions of "zero" and "negative".

On both the 6502 and the 65C02, the behavior of the overflow flag is the same
as if binary mode were active. This makes no sense; but it is actually quite
hard to come up with a meaning of the V flag that makes sense.


