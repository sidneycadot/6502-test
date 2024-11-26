
.PHONY : clean prep-atari-tic-run

default : prep-atari-tic-run

# The TIC_PLATFORM should be one of the platforms supported by sim65, e.g. atari or c64.
TIC_PLATFORM=atari

# Note: CC65 the compiler and assembler also take a '-t" option, which is used for string encoding.
CC65_FLAGS = -O -t ${TIC_PLATFORM}
CA65_FLAGS =    -t ${TIC_PLATFORM}

TIC_OBJS = tic_main.o                              \
           tic_cmd_measurement_test.o              \
           tic_cmd_cpu_test.o                      \
           timing_test_routines.o                  \
           timing_test_report.o                    \
           timing_test_memory.o                    \
           platform_generic.o                      \
           platform_${TIC_PLATFORM}.o

tic_${TIC_PLATFORM}.xex : ${TIC_OBJS}
	cl65 -t ${TIC_PLATFORM} $^ -o $@

tic_main.o : tic_main.c
	cl65 -c $(CC65_FLAGS) $< -o $@

tic_cmd_measurement_test.o : tic_cmd_measurement_test.c
	cl65 -c $(CC65_FLAGS) $< -o $@

tic_cmd_cpu_test.o : tic_cmd_cpu_test.c
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_routines.o : timing_test_routines.c timing_test_routines.h
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_report.o : timing_test_report.c timing_test_report.h
	cl65 -c $(CC65_FLAGS) $< -o $@

timing_test_memory.o : timing_test_memory.c timing_test_memory.h
	cl65 -c $(CC65_FLAGS) $< -o $@

platform_generic.o : platform_generic.s
	cl65 -c $(CA65_FLAGS) $< -o $@

platform_atari.o : platform_atari.s
	cl65 -c $(CA65_FLAGS) $< -o $@

platform_c64.o : platform_c64.s
	cl65 -c $(CA65_FLAGS) $< -o $@

# The 'prep-atari-run' is a convenience target, to copy the 'tic.xex' executable to a location that is easy for tests.
prep-atari-tic-run : tic_atari.xex
	cp tic_atari.xex atari/tic.xex

clean :
	$(RM) *~ *.o *.xex
