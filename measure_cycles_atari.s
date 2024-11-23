
; A function that measures the number of clock cycles taken by short fragments of 6502 code.
; This is the Atari 8-bit specific version of this function.
;
; It can do cycle-exact code fragment timing of code fragments that takes between 0 and 28 (inclusive) cycles
; to execute. Tested code fragments should end in an RTS instruction (not included in the 0..28 cycle count).

                .export _measure_cycles
                .export _dma_and_interrupts_off
                .export _dma_and_interrupts_on

                .include "atari.inc"

                .data

                ; Note: it is important that these are NOT zero pages addresses! Storing data into them should take 4 clock cycles,
                ; to ensure that the RANDOM samples are taken 8 clock cycles apart.

RANDOM_T1:      .res 3  ; Three samples of the RANDOM register taken immediately before the instruction-sequence-under-test.
RANDOM_T2:      .res 3  ; Three samples of the RANDOM register taken immediately after the instruction-sequence-under-test.

                .code

_dma_and_interrupts_off:
                lda     #0
                sta     NMIEN
                sta     IRQEN
                sta     DMACTL
                rts

_dma_and_interrupts_on:
                lda     SDMCTL
                sta     DMACTL
                lda     POKMSK
                sta     IRQEN
                lda     #64
                sta     NMIEN
                rts

_measure_cycles:

                ; NOTE #1: This function strongly depends on cycle-exact Atari-specific behavior.
                ;          Do not change it unless you know what you are doing.
                ;
                ; NOTE #2: This function assumes an absence of interrupts and DMA. The correct environment
                ;          can be set up using the '_dma_and_interrupts_off' subroutine defined above.

                ; The sta WSYNC halts the CPU to a well-defined moment in the 114-cycle scanline sequence that
                ; includes 9 memory refresh cycles, during which the CPU is halted.

                sta     WSYNC                           ; [0 / 56]

                ; We now need to burn precisely 56 clock cycles after the "sta WSYNC" and before the first "lda RANDOM".
                ; That gives us the maximum time window for doing reliable timing, without interference from the
                ; Atari's memory refresh cycles.

                ; We get a 6502 subroutine address as a pointer in X (MSB) and A (MSB).
                ; This subroutine will be called below, between two samplings of the random generator.

                ; The 6502 "rts" instruction will return to the address found on the stack increased by 1.
                ; Calculated the (pointer - 1) value and store it in RANDOM_T1, RANDOM_T1+1 so we can push it later.

                sec                                     ; [2 / 54]
                sbc     #<1                             ; [2 / 52]
                sta     RANDOM_T1                       ; [4 / 48] (Note: we use RANDOM_T1 as temporary storage).

                txa                                     ; [2 / 46]
                sbc     #>1                             ; [2 / 44]
                sta     RANDOM_T1+1                     ; [4 / 40]

                ; First, push return address from the testcode.
                ; Once we're in the test code and reach an RTCS, we should return back to the '@return_from_testcode' address.

                lda     #>(@return_from_testcode - 1)   ; [2 / 38]
                pha                                     ; [3 / 35]
                lda     #<(@return_from_testcode - 1)   ; [2 / 33]
                pha                                     ; [3 / 30]

                ; Push address to jump to the testcode upon RTS.

                lda     RANDOM_T1+1                     ; [4 / 26]
                pha                                     ; [3 / 23]
                lda     RANDOM_T1                       ; [4 / 19]
                pha                                     ; [3 / 16]

                ; We set up a predictable register and flag environment for the test code.

                lda #0                                  ; [2 / 14]
                tax                                     ; [2 / 12]
                tay                                     ; [2 / 10]
                clc                                     ; [2 / 8]
                clv                                     ; [2 / 6]
                cld                                     ; [2 / 4]

                ; Nothing useful left to do. Burn the last 4 cycles using NOPs.

                nop                                     ; [2 / 2]
                nop                                     ; [2 / 0]

                ; Sample RANDOM three times, into RANDOM_T1[0..2], is assumed to take place periodically, at clock cycles T1, T1+8, and T1+16.

                lda     RANDOM
                sta     RANDOM_T1+0     ; Eight rightmost bits of 17-bits LSFR at time T1 + 0 (oldest)
                lda     RANDOM
                sta     RANDOM_T1+1     ; Eight rightmost bits of 17-bits LSFR at time T1 + 8.
                lda     RANDOM
                sta     RANDOM_T1+2     ; Eight rightmost bits of 17-bits LSFR at time T1 + 16 

@execute_testcode_subroutine:

                rts     ; Jump into the 'testcode' subroutine, and return to @return_from_testcode when we reach the RTS that ends it.

@return_from_testcode:

                ; Sample RANDOM three times, into RANDOM_T2[0..2],  is assumed to take place periodically, at clock cycles T2, T2+8, and T2+16.
                lda     RANDOM
                sta     RANDOM_T2+0     ; Eight rightmost bits of 17-bits LSFR at time T2 + 0 (oldest)
                lda     RANDOM
                sta     RANDOM_T2+1     ; Eight rightmost bits of 17-bits LSFR at time T2 + 8.
                lda     RANDOM
                sta     RANDOM_T2+2     ; Eight rightmost bits of 17-bits LSFR at time T2 + 16.

                ; We now have two samplings of the state of RANDOM, each consisting of three bytes taken at 8-clockcycle intervals.
                ; We will now simulate the action of the Atari random generator cycle by cycle, taking the three values RANDOM_T1
                ; in RANDOM_T1 forward in time until we reach a state that is identical to RANDOM_T2.

                ldx     #0                              ; Start the loop with a zero byte interval.

@checkloop:     lda     RANDOM_T1+0                     ; Are the RANDOM_T1 and RANDOM_T2 values equal?
                cmp     RANDOM_T2+0
                bne     @not_equal
                lda     RANDOM_T1+1
                cmp     RANDOM_T2+1
                bne     @not_equal
                lda     RANDOM_T1+2
                cmp     RANDOM_T2+2
                beq     @equal                          ; Success!

@not_equal:     ; The values in RANDOM_T1 and RANDOM_T2 are not yet equal.
                ; We simulate the action of one cycle of the the RANDOM generator, taking RANDOM_T1 one cycle into the future.

                lda     RANDOM_T1+1
                asl
                asl
                asl
                eor     RANDOM_T1+0
                asl

                ror     RANDOM_T1+2
                ror     RANDOM_T1+1
                ror     RANDOM_T1+0

                inx                                     ; Increment X, denoting that the time has gone up by one.

                bne     @checkloop                      ; X has overflowed. We didn't find a match for some reason. Report failure.

                dex                                     ; Return with value 0xffff, denoting failure.
                txa
                rts

@equal:         txa                                     ; Success! Return number of cycles in test code, excluding the rts.
                sec                                     ; Subtract 36 cycles to only measure the start of the test-code subroutine to its RTS.
                sbc     #36
                ldx     #0
@rts:           rts
