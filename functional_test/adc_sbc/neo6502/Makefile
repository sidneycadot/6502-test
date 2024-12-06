
adc_sbc.neo : adc_sbc.o neo6502.cfg
	$(HOME)/Repositories/github/cc65-fork/bin/ld65 -C neo6502.cfg adc_sbc.o -o $@

adc_sbc.o : adc_sbc.s
	$(HOME)/Repositories/github/cc65-fork/bin/ca65 adc_sbc.s -o adc_sbc.o

clean :
	$(RM) adc_sbc.o adc_sbc.neo
