
6502 test
=========

This repository contains programs that try to characterize the behavior of the 6502 processor,
to the extent that it is observable by a 6502 programmer.

Two aspects of the 6502's behavior are considered: functional behavior, and timing behavior.

Functional behavior
-------------------

As microprocessors go, the 6502 is pretty simple. However, to describe in detail what instructions
do is sometimes not entirely trivial.

Undocumented instructions
^^^^^^^^^^^^^^^^^^^^^^^^^

The first thing to note is that out of 256 opcodes, the 'standard' 6502 only defines behavior for
151 of them. That leaves 105 "undocumented opcodes". As it turns out, these opcodes do something,
and a concerted effort by many people has led to a pretty good idea what happens when one of those
is encountered by the processor.

Many of them do nothing; they are essentially NOPs (although some of these 'unoffical' NOPs do memory
access, and take up more than 1 byte). Some of them bring the 6502 to a full stop, because its
internal state machine gets stuck; these opcodes are sometimes referred to as the "JAM" instructions. 

Other undocumented instructions perform well-defined, if sometimes weird operations, by mixing-and-
matching behavioral elements of other instructions in unintended ways.

Lastly, there are a handful instructions that do /not/ perform a well-defined function; their behavior
turns out to be somewhat unpredictable, and varies from machine to machine, with temperature, and/or
with the presence of signals to the 6502 at certain stages during their execution.

Undocumented instructions in the 6502 are an interesting topic; the reader is referred to Google
to find more information on them, most notably the "NMOS 6510 Unintended Opcodes" document, that
tries to present everything that is known about them in one place.

Documented instructions with undocumented behavior: ADC and SBC
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are two documented instructions the behavior of which is not fully specified in some
circumstances: the Add-With-Carry (ADC) and Subtract-With-Carry (SBC) instructions.

These instructions can operate in two modes: "binary mode" and "decimal mode". The current
state of the D ('decimal') bit of the 6502 controls which mode will be used when such an
instruction is encountered.

In binary mode (D=0), the operation of the instruction, including its effect on the 6502's
N/V/Z/C flags, is fully documented and pretty straightforward.

Things change, however, in decimal mode (D=1). In this mode, the behavior of the instructions
is only specified in case the A register and the operand are both "BCD" numbers. Even then,
only the effect on the accumulator itself and the carry (C) bit is fulle specified.

As with the undocumented instructions, the behavior of ADC and SBC has been thoroughly
studied and described. As it turns out, the behavior is fully deterministic, but very hard
to understand. The nice symmetry between ADC and SBC in decimal mode is broken; there
are strange differences that come to light when re-implementing the behavior.

To some extent, later versions of the 6502, such as the 65C02, tried to rectify this situation,
by at least defining meaningful semantics to the N and Z flags (in addition to the C flag)
when ADC and SBC are used in decimal mode. Still, the behavior when non-BCD numbers are
encountered, and the behavior of the overflow (V) flag even when BCD numbers are processed,
are quite strange, and not normally useful.

More information about the ADC and SBC instructions can be found in this repository, in the
functional_tests/adc_sbc subdirectory.

Timing behavior
---------------

Another interesting aspect of the 6502's behavior is timing. One of the neat things about
the 6502 is that it doesn't have fancy features like caching, pipelining, branch prediction,
or pre-fetching. All those things are nice for performance, but _not_ having them also has
a distinct advantage: it is possible to use _cycle counting_ to precisely predict how long
a given piece of code will run.

This is because every opcode has a well-defined number of clock cycles that it takes to
execute. Sometimes, this number of cycles depends on the content of registers, or the
precise location of the code, or data, in memory. When all these things are accounted for
though, the duration of a given instruction is fully predictable, with relative ease.

