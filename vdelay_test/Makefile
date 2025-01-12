
.PHONY : default run-test clean

default : run-test

run-test : test_vdelay.prg
	sim65 test_vdelay.prg

test_vdelay.prg : test_vdelay.c time_vdelay.s vdelay.s sim6502_with_align.cfg
	cl65 -C sim6502_with_align.cfg -t sim6502 -O test_vdelay.c time_vdelay.s vdelay.s -o test_vdelay.prg

clean :
	$(RM) test_vdelay.prg
