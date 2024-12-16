
.PHONY : clean

test_sha_absy.xex :test_sha_absy.c sha_absy.s
	cl65 -t atari $^ -o $@

clean:
	$(RM) *.o test_sha_absy.xex
