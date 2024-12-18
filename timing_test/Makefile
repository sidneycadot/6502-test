
.PHONY : default atari c64 sim6502 sim65c02 neo all prep-atari prep-c64 prep-neo clean

# To keep all temporary (intermediate) files, enable the ".SECONDARY" target.
#.SECONDARY:

default:
	@echo
	@echo "Please provide a build target, e.g."
	@echo
	@echo "  make atari"
	@echo "  make c64"
	@echo "  make sim6502"
	@echo "  make sim65c02"
	@echo "  make neo"
	@echo "  make all"
	@echo

atari :
	$(MAKE) tic.xex          TIC_PLATFORM=atari    TIC_PLATFORM_UPPERCASE=ATARI    TIC_SIM65_TARGET=atari    TIC_EXECUTABLE_NAME=tic.xex          TIC_CPU=6502

c64 :
	$(MAKE) tic.prg          TIC_PLATFORM=c64      TIC_PLATFORM_UPPERCASE=C64      TIC_SIM65_TARGET=c64      TIC_EXECUTABLE_NAME=tic.prg          TIC_CPU=6502

sim6502 :
	$(MAKE) tic_sim6502.prg  TIC_PLATFORM=sim6502  TIC_PLATFORM_UPPERCASE=SIM6502  TIC_SIM65_TARGET=sim6502  TIC_EXECUTABLE_NAME=tic_sim6502.prg  TIC_CPU=6502

sim65c02 :
	$(MAKE) tic_sim65c02.prg TIC_PLATFORM=sim65c02 TIC_PLATFORM_UPPERCASE=SIM65C02 TIC_SIM65_TARGET=sim65c02 TIC_EXECUTABLE_NAME=tic_sim65c02.prg TIC_CPU=65C02

neo :
	$(MAKE) tic.neo          TIC_PLATFORM=neo      TIC_PLATFORM_UPPERCASE=NEO      TIC_SIM65_TARGET=none     TIC_EXECUTABLE_NAME=tic_raw.out      TIC_CPU=65C02 \
	        LD65_FLAGS_EXTRA="--start-addr 0x0200 --ld-args --define,__STACKSTART__=0xfc00"                                                                     \
	        TIC_OBJS_EXTRA="neo6502_glue_neo.o"

all  : atari c64 sim6502 sim65c02 neo

# A convenience target to facilitate rapid development for the Atari platform.
# Set up an 'atari' directory on your PC that can easily be accessed from your Atari for optimal results.
prep-atari : atari
	cp tic.xex atari/

# A convenience target to facilitate rapid development for the Atari platform.
# Set up a 'c64' directory on your PC that can easily be accessed from your C64 for optimal results.
prep-c64 : c64
	cp tic.prg c64/

# A convenience target to facilitate rapid development for the Neo6502 platform.
# Set up a 'neo' directory on your PC that can easily be accessed from your Neo6502 for optimal results.
prep-neo : neo
	cp tic.neo neo/

# Define linker, compiler, and assembler flags.
# Note that the C65 compiler and assembler take a -t option, which is used for string encoding.
# This means that their output files are also target-specific.
CC65_FLAGS := -O -t ${TIC_SIM65_TARGET} -DTIC_PLATFORM_${TIC_PLATFORM_UPPERCASE} -DCPU_${TIC_CPU}
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
	$(RM) *~ *.o tic.xex tic.prg tic_raw.out tic_sim6502.prg tic_sim65c02.prg tic.neo
