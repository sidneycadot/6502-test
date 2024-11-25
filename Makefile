
.PHONY : clean prep-atari-tic-run

default : prep-atari-tic-run

TARGET_PLATFORM=atari

# Note: the compiler and assembler also take a _'-t" option, which is used for string encoding.
CC65_FLAGS = -O -t ${TARGET_PLATFORM}
CA65_FLAGS =    -t ${TARGET_PLATFORM}

TIC_OBJS = tic_main.o                              \
           tic_cmd_measurement_test.o              \
           tic_cmd_cpu_test.o                      \
           timing_test_routines.o                  \
           timing_test_report.o                    \
           timing_test_memory.o                    \
           measure_cycles_zp_safe.o                \
           measure_cycles_${TARGET_PLATFORM}.o

tic.xex : ${TIC_OBJS}
	cl65 -t  ${TARGET_PLATFORM} $^ -o $@

tic_main.o : tic_main.c
	cl65 -c $(CC65_FLAGS) $< -o $@

tic_cmd_measurement_test.o : tic_cmd_measurement_test.c
	cl65 -c $(CC65_FLAGS) $< -o $@

tic_cmd_cpu_test.o : tic_cmd_cpu_test.c
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_routines.o : timing_test_routines.c timing_test_routines.h  measure_cycles.h
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_report.o : timing_test_report.c timing_test_report.h
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_memory.o : timing_test_memory.c timing_test_memory.h
	cl65 -c $(CC65_FLAGS) $< -o $@

measure_cycles_zp_safe.o : measure_cycles_zp_safe.s
	cl65 -c $(CA65_FLAGS) $< -o $@

measure_cycles_atari.o : measure_cycles_atari.s
	cl65 -c $(CA65_FLAGS) $< -o $@

# The 'prep-atari-run' is a convenience target, to copy the 'tic.xex' executable to a location that is easy for tests.
prep-atari-tic-run : tic.xex
	cp tic.xex atari/

clean :
	$(RM) *~ *.o tic.xex
