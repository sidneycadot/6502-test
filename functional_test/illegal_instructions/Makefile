
.PHONY : default clean

default : test_sha.prg test_sha.xex

test_sha.prg :test_sha.c sha_absy.s
	cl65 -t c64 $^ -o $@

test_sha.xex :test_sha.c sha_absy.s
	cl65 -t atari $^ -o $@

clean:
	$(RM) *.o test_sha.prg test_sha.xex
