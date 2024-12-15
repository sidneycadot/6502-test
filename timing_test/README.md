TIC, the 6502/65C02 instruction cycle-count tester
==================================================

The TIC program tests the instruction timing (i.e., clock-cycle counts) of the
6502 and 65C02 instructions. For the 6502, both documented and undocumented
opcodes are supported. For the 65C02, all 254 'regular' opcodes are supported
(the 65C02 STP and WAI instructions have an indefinite cycle count).

The program was validated on physical hardware (an Atari 800XL, a Commodore 64,
and a Neo6502 board featuring a WDC 65C02 chip); those show zero errors, providing
strong evidence that the program's clock cycle test routines are correct.

With that fact in mind, the program can serve as a validation reference for
emulators that try to emulate an entire machine, or only the 6502/65C02 processor.

ASSEMBLY SUPPORT ROUTINES
-------------------------

TIC is written in C, but its core measurement functionality depends on several
routines written in 6502 assembly. Some of these are platform-specific:

* measure_cycles()           Measure the number of clock cycles needed to execute a short
                             sequence of 6502 instructions.
                               The implementation of this routine is platform-specific.
                             On the Atari, the (somewhat unfortunately named) RANDOM
                             register is used, as it provides a predictable, cycle-precise
                             progression through a sequence of values with period 131,071.
                               On the C64, a CIA timer is used for this purpose.
                             The length of instruction sequences that can be timed varies
                             between hardware platforms. The current implementation on the
                             Atari, for example, can only measure instruction sequences
                             up to 27 clock cycles, because interference with the
                             continuously operation DRAM refresh process (that halts the
                             processor in a regular pattern during 9 cycles out of 114
                             cycles). The Atari's measurement routine is hard-synchronized
                             with this 114-period cycle, and measurement is done in a time
                             window where the CPU is not halted.

* set_irq_vector_address()   The BRK function initiates a "software IRQ" operation, where
                             the 6502 will jump through its IRQ interrupt vector at address
                             (0xfffe, 0xffff). On many platforms, the OS offers the
                             possibility of transferring control to user code, through a
                             vector in RAM. On other (often emulated) systems, it may be
                             possible to write to the IRQ vector directly. In either case,
                             this routine sets up the appropriate IRQ vector and returns the
                             previous value, so it can be restored.

A variant of the measure_cycles() routine is needed to accommodate tests that can write to
the zero page, which may be in use by the 6502 machine's operating system. This function
depends on the platform dependent measure_cycles() routine, but is itself platform
independent. It is a drop-in replacement for the standard 'measure_cycles' for cases when
this zero-page preservation behavior is needed:

* measure_cycles_zp_safe()   Save the contents of the zero page, execute measure_cycles(),
                             then restore the content of the zero page.

ASSUMPTIONS MADE BY TIC
-----------------------

* The program assumes that the 6502 and 65C02 instructions work 100% correctly, other than
  that their clock cycle counts may be off.

* Later tests assume that the clock cycle counts of 6502/65C02 instructions that were timed
  by earlier tests for simpler instructions are correct.

ADDING SUPPORT FOR NEW PLATFORMS
--------------------------------

Currently, the TIC Makefile supports several build targets curresponding to several target
hardware platforms. The platforms included are the stock Atari 8-bit machines,
stock Commodore 64 machines, and the Neo6502 with a patched version of the Morpheus
firmware.

Support for new 6502/65C02 hardware platforms can be added to TIC, which is useful both to
further establish the trustworthiness of the tests implemented in TIC itself, but also to
stress-test emulators for those plaforms. Emulating a processor in a cycle-exact way is a
pretty challenging task, especially when the measurement method relies oncycle-exact
behavior by the non-CPU hardware in the system.

The most important technical challenge when it comes to adding a hardware platform to TIC
is that a way needs to be found that allows 100% reliable clock-cycle exact timing of short
fragments of 6502/65C02 code (up to about 25 cycles).

This is not always trivial: for the Atari machines, this relies on a clever trick to use its
built-in 'hardware random' register to count cycles, taking into account the fact that this
counter is still counting while the CPU itself is halted for RAM refresh cycles.
  For the Neo6502 platform, this required a patch to the RP2040 firmware to make an 8-bit
clock-cycle counter available in the WDC6502 memory.

COMPATIBILITY NOTES
-------------------

6502 undocumented instructions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On the Atari, it was found that the TIC program crashes on five of the undocumented 6502
opcodes:

Opcode 0x93 :     SHA (zpage,Y)
Opcode 0x9f :     SHA abs,Y
Opcode 0x9e :     SHX abs,Y
Opcode 0x9c :     SHY abs,X
Opcode 0x9b :     TAS abs,Y

The reason for these crashes is not currently understood. If we replace these opcodes with
documented counterparts with well-defined behavior, the crashes disappear. This suggests
that the problems may be due to undefined behavior beyond what is generally known and/or
assumed for these opcodes. It may be that TIC, by running through many variations of these
opcodes in terms of opcode address, address values, and index register values, encounters
cases that trigger behavior that is not currently described. We'll see.

ADC/SBC instructions on the 65C02
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The 65C02 changes (fixes) the behavior of the C and V flags when doing addition or subtraction
(ADC or SBC) in decimal mode, at the cost of an extra clock cycle. This behavior is currently
not tested, as all tests are run with decimal mode disabled.
