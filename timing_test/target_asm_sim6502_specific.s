
; SIM65 currently provides no way to do clock cycle measurements.
; Adding support for this is being discusssed on the CC65 issue tracker: https://github.com/cc65/cc65/issues/2355

                .export _measure_cycles

                .bss

TIMER_T1:       .res 2
TIMER_T2:       .res 2

                .code

_measure_cycles:

                php                                     ; [3 / 53] Save the status bits (in particular, the P register).
                cld                                     ; [2 / 51] Make sure we're not in decimal mode.

                sec                                     ; [2 / 49]
                sbc     #<1                             ; [2 / 47]
                sta     TIMER_T1                        ; [4 / 43] (Note: we use RANDOM_T1 as temporary storage).

                txa                                     ; [2 / 41]
                sbc     #>1                             ; [2 / 39]
                sta     TIMER_T1+1                      ; [4 / 35]

                ; First, push return address from the testcode.
                ; Once we're in the test code and reach an RTCS, we should return back to the '@return_from_testcode' address.

                lda     #>(@return_from_testcode - 1)   ; [2 / 33]
                pha                                     ; [3 / 30]
                lda     #<(@return_from_testcode - 1)   ; [2 / 28]
                pha                                     ; [3 / 25]

                ; Push address to jump to the testcode upon RTS.

                lda     TIMER_T1+1                      ; [4 / 21]
                pha                                     ; [3 / 18]
                lda     TIMER_T1                        ; [4 / 14]
                pha                                     ; [3 / 11]

                lda     #0
                sta     $ffc1				; Select clock cycle timer.

                sta     $ffc0				; Latch counter values.
		lda     $ffc2				; Copy lowest 16 bits of latched clock cycle counter.
		sta     TIMER_T1
 		lda	$ffc3
                sta     TIMER_T1+1

@execute_testcode_subroutine:

                rts     ; Jump into the 'testcode' subroutine, and return to @return_from_testcode when we reach the RTS that ends it.

@return_from_testcode:

                sta     $ffc0				; Latch counter values.
		lda     $ffc2				; Copy lowest 16 bits of latched clock cycle counter.
		sta     TIMER_T2
 		lda	$ffc3
                sta     TIMER_T2+1

                ; T2 -= T1

                sec
                lda     TIMER_T2
                sbc     TIMER_T1
                sta     TIMER_T2

                lda     TIMER_T2+1
                sbc     TIMER_T1+1
                sta     TIMER_T2+1

                ; T2 -= 32

                sec
                lda     TIMER_T2
                sbc     #<32
                sta     TIMER_T2

                lda     TIMER_T2+1
                sbc     #>32
                sta     TIMER_T2+1

                ;

                lda     TIMER_T2
                ldx     TIMER_T2 + 1

@done:          plp                                     ; Restore the flags that were present on entry.
                rts
