
.PHONY : clean

adcsbc.xex : adcsbc.c testfunc.s
	cl65 -t atari $^ -o $@

clean :
	$(RM) adcsbc.xex
