
.PHONY : clean

adc_sbc.xex : adc_sbc.s
	cl65 -t atari $^ -o $@

clean :
	$(RM) adc_sbc.xex
