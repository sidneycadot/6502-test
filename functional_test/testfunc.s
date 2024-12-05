
            .export _A_VALUE
            .export _P_VALUE
            .export _TABLE
            .export _testfunc

_testfunc   := testfunc
_A_VALUE    := A_VALUE
_P_VALUE    := P_VALUE
_TABLE      := TABLE

PTR         := $CD

            .bss

A_VALUE:    .res    1       ; Value transferred to A before the ADC/SBC.
P_VALUE:    .res    1       ; Value transferred to P before the ADC/SBC  (bits 4 and 5 are ignored).
OPERAND:    .res    1
TABLE:      .res    1024    ; We need 256 * 4 bytes to store results: 256 operand values * {ADC A/P, SBC A/P}.

            .code

testfunc:
            lda     #<TABLE
            sta     PTR
            lda     #>TABLE
            sta     PTR+1

            lda     #0
            sta     OPERAND
@loop:
            ; Save old status bits (for the I flag, mostly).

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
