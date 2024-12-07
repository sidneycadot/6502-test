
.PHONY : default clean show

CFLAGS = -W -Wall -O3

# The datafiles were originally recorded in hardware (Atari 800XL and Neo6502), but can now be re-generated
# by the "make_reference_files" program, which reproduces them perfectly.
DATAFILES = adc_sbc_6502.dat adc_sbc_65c02.dat

default : test_adc_and_sbc $(DATAFILES)
	./test_adc_and_sbc

show : testcases.npy $(DATAFILES)
	./show_testcases.py

$(DATAFILES) : make_reference_files MD5SUM
	./make_reference_files
	md5sum -c MD5SUM

make_reference_files : make_reference_files.c adc_and_sbc.c adc_and_sbc.h

test_adc_and_sbc : test_adc_and_sbc.c adc_and_sbc.c adc_and_sbc.h

testcases.npy : preprocess.py $(DATAFILES)
	./preprocess.py

clean :
	$(RM) test_adc_and_sbc make_reference_files testcases.npy *~ $(DATAFILES)
