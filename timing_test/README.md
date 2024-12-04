TIC, the 6502 instruction cycle-count tester
============================================

The TIC program tests the instruction timing (i.e., clock-cycle counts) of the
151 documented 6502 instructions.

The program was validated on physical hardware (for example, an Atari 800XL and
a Commodore 64); those show zero errors, providing strong evidence that the
program's clock cycle test routines are correct.

With that fact in mind, the program can serve as a validation reference for
emulators that try to emulate an entire machine, or only the 6502 processor.

ASSEMBLY SUPPORT ROUTINES
-------------------------

TIC is written in C, but its core measurement functionality depends on several
routines written in 6502 assembly. Some of these are platform-specific:

* dma_and_interrupts_off()   This creates an environment where the measure_cycles()
                             and measure_cycles_zp_safe() routines can do their work.
                             On most hardware, this is a matter of disabling video DMA
                             and interrupts; hence the name.

* dma_and_interrupts_on()    Restore a "normal" environment, where DMA, interrupts, and
                             any other timing disturbances are once again allowed.

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

* The program assumes that the 6502 instructions work 100% correctly, other than that their
  clock cycle counts may be off.

* Later tests assume that the clock cycle counts of 6502 instructions that were timed by
  earlier tests for simpler instructions are correct.

COMPATIBILITY NOTE
------------------

The 65C02 changes (fixes) the behavior of the C and V flags when doing addition or subtraction
(ADC or SBC) in decimal mode, at the cost of an extra clock cycle. This behavior is currently
not tested, as all tests are run with decimal mode disabled. We will add explicit tests
for this once we have a 65C02 system available.
