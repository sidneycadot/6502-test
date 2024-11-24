
.PHONY : clean prep-atari-tic-run

default : prep-atari-tic-run

tic.xex : test_instruction_clocks.c measure_cycles_zp_safe.s measure_cycles_atari.s
	cl65 -t atari test_instruction_clocks.c measure_cycles_atari.s measure_cycles_zp_safe.s -o tic.xex

# The 'prep-atari-run' is a convenience target, to copy the 'tic.xex' executable
# to a location that is easy for tests.
prep-atari-tic-run : tic.xex
	cp tic.xex atari/

clean :
	$(RM) tic.xex
