TIC, the 6502 instruction cycle-count tester
============================================

The TIC program tests the instruction timing (i.e., clock-cycle counts) of
the 151 documented 6502 instructions.

Te program was validated on physical hardware (for example, an Atari 800XL and a
Commodore 64); those show zero errors.

With that fact in mind, the program can serve as a validation reference for emulators
that try to emulate an entire machine, or only the 6502 processor.

The program depends on the availability of four external functions. Three of those
are specific to the hardware platform on which the program is run:

* dma_and_interrupts_off()   This create an environment where the measure_cycles()
                             and measure_cycles_zp_safe() functions can do their work.
                             On most hardware, this is a matter of disabling video DMA
                             and interrupts; hence the name.

* dma_and_interrupts_on()    Restore a "normal" environment, where DMA, interrupts, and
                             any other timing disturbances are once again allowed.

* measure_cycles()           Measure the number of clock cycles needed to execute a short
                             sequence of 6502 instructions.
                               The length of instruction sequences that can be timed varies
                             between hardware platforms. The current implementation on the
                             Atari, for example, can only measure instruction sequences
                             reliably up to 27 clock cycles.

A fourth external function is needed to accommodate tests that can write to the zero page,
which may be in use by the 6502 machine's operating system. This function depends on the
platform dependent measure_cycles() routine, but is itself platform independent. It is a
drop-in replacement for the standard 'measure_cycles' for cases when this zero-page
preservation behavior is needed:

* measure_cycles_zp_safe()   Save the contents of the zero page, execute measure_cycles(),
                             then restore the content of the zero page.

ASSUMPTIONS MADE BY TIC
-----------------------

* The program assumes that the 6502 instructions work correctly, other than that their clock
  cycle counts may be off.

* Later tests assume that the clock cycle counts of 6502 instructions that were timed by
  earlier tests for simpler instructions are correct.

COMPATIBILITY NOTE
------------------

The 65C02 changes (fixes) the behavior of the C and V flags when doing addition or subtraction
(ADC or SBC) in decimal mode, at the cost of an extra clock cycle. This behavior is currently
not tested, as all tests are run with decimal mode disabled. We will add explicit tests
for this once we have a 65C02 system available.
