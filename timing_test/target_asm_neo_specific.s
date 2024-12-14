
; A function that measures the number of clock cycles taken by short fragments of 6502 code.
; This is the Atari 8-bit specific version of this function.
;
; It can do cycle-exact code fragment timing of code fragments that takes between 0 and 28 (inclusive) cycles
; to execute. Tested code fragments should end in an RTS instruction (not included in the 0..28 cycle count).

                .export _measure_cycles

                .bss

TMP:            .res 2

                .code

_measure_cycles:

                php                                     ; Save the status bits (in particular, the P register).
                cld                                     ; Make sure we're not in decimal mode.

                sec                                     ;
                sbc     #<1                             ;
                sta     TMP                             ;

                txa                                     ;
                sbc     #>1                             ;
                sta     TMP+1                           ;

                ; First, push return address from the testcode.
                ; Once we're in the test code and reach an RTS, we should return back to the '@return_from_testcode' address.

                lda     #>(@return_from_testcode - 1)   ; [2 / 33]
                pha                                     ; [3 / 30]
                lda     #<(@return_from_testcode - 1)   ; [2 / 28]
                pha                                     ; [3 / 25]

                ; Push address to jump to the testcode upon RTS.

                lda     TMP+1                           ; [4 / 21]
                pha                                     ; [3 / 18]
                lda     TMP                             ; [4 / 14]
                pha                                     ; [3 / 11]

@sample_before: ; Sample timer.

                lda     $ff80
                sta     TMP

@execute_testcode_subroutine:

                rts     ; Jump into the 'testcode' subroutine, and return to @return_from_testcode when we reach the RTS that ends it.

@return_from_testcode:

                ; Sample timer.
                lda     $ff80

                sec
                sbc     TMP
                sec
                sbc     #20

                ldx     #0

                plp
                rts
