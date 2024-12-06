
                .include "atari.inc"

                .export _main

PTR             := $CD

TABLE_SIZE = 1024

                .rodata

FILENAME:       .byte   "D:adc_sbc_6502.dat", $9B

                .bss

P_VALUE:        .res    1       ; Value transferred to P before the ADC/SBC  (bits 4 and 5 are ignored).
A_VALUE:        .res    1       ; Value transferred to A before the ADC/SBC.
OPERAND:        .res    1
TABLE:          .res    TABLE_SIZE ; We need 256 * 4 bytes to store results: 256 operand values * {ADC A/P, SBC A/P}.

                .code

_main:          ; Open file.

                ldx     #$10
                lda     #3
                sta     ICCOM,x
                lda     #<FILENAME
                sta     ICBAL,x
                lda     #>FILENAME
                sta     ICBAH,x
                lda     #8
                sta     ICAX1,x
                lda     #0
                sta     ICAX2,x
                jsr     CIOV
                bmi     OOPS

                ; Start of A, P loop

                lda     #0
                sta     P_VALUE
                sta     A_VALUE

P_LOOP:         ; Check if we want to test the current value of P_VALUE.

                lda     P_VALUE
                and     #$f6
                cmp     #$32
                bne     SKIP_P_VALUE

A_LOOP:         ; Perform the test for current A, P values.

                jsr     TEST_ADC_SBC

                ; Save data to disk.

                ldx     #$10
                lda     #11
                sta     ICCOM,x
                lda     #<TABLE
                sta     ICBAL,x
                lda     #>TABLE
                sta     ICBAH,x
                lda     #<TABLE_SIZE
                sta     ICBLL,x
                lda     #>TABLE_SIZE
                sta     ICBLH,x
                jsr     CIOV
                bmi     OOPS

                inc     A_VALUE
                bne     A_LOOP

SKIP_P_VALUE:   inc     P_VALUE
                bne     P_LOOP

                ; Close file.

                ldx     #$10
                lda     #12
                sta     ICCOM,x
                jsr     CIOV
                bmi     OOPS

OOPS:           rts

TEST_ADC_SBC:   lda     #<TABLE
                sta     PTR
                lda     #>TABLE
                sta     PTR+1

                lda     #0
                sta     OPERAND

@loop:          ; Save old status bits (for the I flag, mostly).

                php

                ; Do ADC test.

                lda     P_VALUE
                pha
                lda     A_VALUE
                plp

                adc     OPERAND

                php
                ldy     #0
                sta     (PTR),y             ; Store ADC A result
                pla
                ldy     #1
                sta     (PTR),y             ; Store ADC P resu;t

                ; Do SBC test

                lda     P_VALUE
                pha
                lda     A_VALUE
                plp

                sbc     OPERAND

                php
                ldy     #2
                sta     (PTR),y             ; Store SBC A result
                pla
                ldy     #3
                sta     (PTR),y             ; Store SBC P result

                ; Restore old status bits (for the I flag).

                plp

                ; Increment PTR by 4.

                clc
                lda     PTR
                adc     #<4
                sta     PTR
                lda     PTR+1
                adc     #>4
                sta     PTR+1

                ; Proceed to next OPERAND value.

                inc     OPERAND
                bne     @loop

                rts
