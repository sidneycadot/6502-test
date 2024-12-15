
.PHONY : default atari c64 neo sim65 all prep-atari prep-c64 clean

# To keep all temporary (intermediate) files, enable the ".SECONDARY" target.
#.SECONDARY:

default:
	@echo
	@echo "Please provide a build target, e.g."
	@echo
	@echo "  make atari"
	@echo "  make c64"
	@echo "  make neo"
	@echo "  make sim65"
	@echo "  make all"
	@echo

all  : atari c64 neo sim65

atari :
	$(MAKE) tic.xex       TIC_PLATFORM=atari TIC_PLATFORM_UPPERCASE=ATARI TIC_SIM65_TARGET=atari   TIC_EXECUTABLE_NAME=tic.xex

c64 :
	$(MAKE) tic.prg       TIC_PLATFORM=c64   TIC_PLATFORM_UPPERCASE=C64   TIC_SIM65_TARGET=c64     TIC_EXECUTABLE_NAME=tic.prg

neo :
	$(MAKE) tic.neo       TIC_PLATFORM=neo   TIC_PLATFORM_UPPERCASE=NEO   TIC_SIM65_TARGET=none    TIC_EXECUTABLE_NAME=tic_raw.out \
	        LD65_FLAGS_EXTRA="--start-addr 0x0200 --ld-args --define,__STACKSTART__=0xfc00"                                        \
	        TIC_OBJS_EXTRA="neo6502_glue_neo.o"

sim65 :
	$(MAKE) tic_sim65.prg TIC_PLATFORM=sim65 TIC_PLATFORM_UPPERCASE=SIM65 TIC_SIM65_TARGET=sim6502 TIC_EXECUTABLE_NAME=tic_sim65.prg

# A convenience target to facilitate rapid development for the Atari platform.
# Set up an 'atari' directory on your PC that can easily be accessed from your Atari for optimal results.
prep-atari : atari
	cp tic.xex atari/tic.xex

# A convenience target to facilitate rapid development for the Atari platform.
# Set up an 'atari' directory on your PC that can easily be accessed from your Atari for optimal results.
prep-c64 : c64
	cp tic.prg c64/tic.prg

# Define linker, compiler, and assembler flags.
# Note that the C65 compiler and assembler take a -t option, which is used for string encoding.
# This means that their output files are also target-specific.
CC65_FLAGS := -O -t ${TIC_SIM65_TARGET} -DTIC_PLATFORM_${TIC_PLATFORM_UPPERCASE} -DCPU_6502
CA65_FLAGS :=    -t ${TIC_SIM65_TARGET}
LD65_FLAGS :=    -t ${TIC_SIM65_TARGET} ${LD65_FLAGS_EXTRA}

# Note: object files are not interchangeable between targets,
# since they are compiled/assembled with the target-specific -t flag.
TIC_OBJS := tic_main_${TIC_PLATFORM}.o                            \
            tic_cmd_measurement_test_${TIC_PLATFORM}.o            \
            tic_cmd_cpu_test_${TIC_PLATFORM}.o                    \
            timing_test_routines_${TIC_PLATFORM}.o                \
            timing_test_measurement_${TIC_PLATFORM}.o             \
            timing_test_memory_${TIC_PLATFORM}.o                  \
            target_asm_generic_${TIC_PLATFORM}.o                  \
            target_asm_${TIC_PLATFORM}_specific_${TIC_PLATFORM}.o \
            target_${TIC_PLATFORM}_specific_${TIC_PLATFORM}.o     \
	    ${TIC_OBJS_EXTRA}

tic.neo : tic_raw.out
	@ # The Neo6502 uses CC65 target "none" and outputs a bare executable image.
	@ # We need to prepend a header get to an executable image.
	./make_neo_executable.py $< $@

# Link the executable.
# Note that we call cl65 as linker, not ld65.
# The cl65 driver script knows to include all target-specific libraries.

${TIC_EXECUTABLE_NAME} : ${TIC_OBJS}
	cl65 $(LD65_FLAGS) $^ -o $@

# Compile a platform-specific C file to a platform-specific assembly file.
target_${TIC_PLATFORM}.s : target_${TIC_PLATFORM}.c
	cc65 $(CC65_FLAGS) $< -o $@

# Compile a generic C file to a platform-specific assembly file.
%_${TIC_PLATFORM}.s : %.c
	cc65 $(CC65_FLAGS) $< -o $@

# Assemble a generic assembly file to a platform-specific object file.
%_${TIC_PLATFORM}.o : %.s
	ca65 $(CA65_FLAGS) $< -o $@

# Assemble a platform-specific assembly file to a platform-specific object file.
%_${TIC_PLATFORM}.o : %_${TIC_PLATFORM}.s
	ca65 $(CA65_FLAGS) $< -o $@

clean :
	$(RM) *~ *.o tic.xex tic.prg tic_raw.out tic.neo
