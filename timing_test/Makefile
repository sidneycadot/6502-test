
.PHONY : atari c64 all prep-atari

# To keep all temporary (intermediate) files, enable the ".SECONDARY" target.
#.SECONDARY:

default:
	@echo
	@echo "Please provide a build target, e.g."
	@echo
	@echo "  make atari"
	@echo "  make c64"
	@echo "  make sim65"
	@echo "  make all"
	@echo

all  : atari c64 sim65

atari :
	$(MAKE) tic_atari.xex TIC_TARGET=atari TIC_TARGET_UPPERCASE=ATARI TIC_TARGET_EXTENSION=xex

c64 :
	$(MAKE) tic_c64.prg TIC_TARGET=c64 TIC_TARGET_UPPERCASE=C64 TIC_TARGET_EXTENSION=prg

sim65 :
	$(MAKE) tic_sim6502.prg TIC_TARGET=sim6502 TIC_TARGET_UPPERCASE=SIM6502 TIC_TARGET_EXTENSION=prg

# A convenience target to facilitate rapid development for the Atari.
prep-atari : atari
	cp tic_atari.xex atari/tic.xex

# Define linker, compiler, and assembler flags.
# Note that the C65 compiler and assembler take a -t option, which is used for string encoding.
# This means that their output files are also target-specific.
CC65_FLAGS := -O -t ${TIC_TARGET} -DTIC_TARGET_${TIC_TARGET_UPPERCASE}
CA65_FLAGS :=    -t ${TIC_TARGET}
LD65_FLAGS :=    -t ${TIC_TARGET}

# Note: object files are not interchangeable between targets,
# since they are compiled/assembled with the target-specific -t flag.
TIC_OBJS := tic_main_${TIC_TARGET}.o                    \
            tic_cmd_measurement_test_${TIC_TARGET}.o    \
            tic_cmd_cpu_test_${TIC_TARGET}.o            \
            timing_test_routines_${TIC_TARGET}.o        \
            timing_test_measurement_${TIC_TARGET}.o     \
            timing_test_memory_${TIC_TARGET}.o          \
            target_asm_${TIC_TARGET}.o                  \
            target_asm_generic_${TIC_TARGET}.o          \
            target_${TIC_TARGET}.o

# Link the final executable.
# Note that we call cl65 as linker, not ld65.
# The cl65 driver script knows to include all target-specific libraries.
tic_${TIC_TARGET}.${TIC_TARGET_EXTENSION} : ${TIC_OBJS}
	cl65 $(LD65_FLAGS) $^ -o $@

# Compile a target-specific C file to a target-specific assembly file.
target_${TIC_TARGET}.s : target_${TIC_TARGET}.c
	cc65 $(CC65_FLAGS) $< -o $@

# Compile a generic C file to a target-specific assembly file.
%_${TIC_TARGET}.s : %.c
	cc65 $(CC65_FLAGS) $< -o $@

# Assemble a generic assembly file to a target-specific object file.
%_${TIC_TARGET}.o : %.s
	ca65 $(CA65_FLAGS) $< -o $@

# Assemble a target-specific assembly file to a target-specific object file.
%_${TIC_TARGET}.o : %_${TIC_TARGET}.s
	ca65 $(CA65_FLAGS) $< -o $@

clean :
	$(RM) *~ *.o *.xex *.prg target_atari.s target_c64.s
