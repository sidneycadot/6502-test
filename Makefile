
.PHONY : clean prep-atari-tic-run

default : prep-atari-tic-run

CC65_FLAGS=-O

tic.xex : test_instruction_clocks.o timing_test_routines.o timing_test_report.o timing_test_memory.o measure_cycles_zp_safe.o measure_cycles_atari.o 
	cl65 -t atari $^ -o $@

test_instruction_clocks.o : test_instruction_clocks.c
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_routines.o : timing_test_routines.c timing_test_routines.h  measure_cycles.h
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_report.o : timing_test_report.c timing_test_report.h
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_memory.o : timing_test_memory.c timing_test_memory.h
	cl65 -c $(CC65_FLAGS) $< -o $@

measure_cycles_zp_safe.o : measure_cycles_zp_safe.s
	cl65 -c $< -o $@

measure_cycles_atari.o : measure_cycles_atari.s
	cl65 -c $< -o $@


# The 'prep-atari-run' is a convenience target, to copy the 'tic.xex' executable
# to a location that is easy for tests.
prep-atari-tic-run : tic.xex
	cp tic.xex atari/

clean :
	$(RM) *~ *.o tic.xex
