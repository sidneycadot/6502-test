
            .export _BEFORE_P
            .export _BEFORE_A
            .export _TABLE

_BEFORE_P   := BEFORE_P
_BEFORE_A   := BEFORE_A
_TABLE      := TABLE

PTR         := $CD

            .bss

BEFORE_P    .res    1
BEFORE_A    .res    1
OPERAND     .res    1
TABLE       .res    1024

            .code
_testfunc:

            lda     #<TABLE
            sta     PTR
            lda     #>TABLE
            sta     PTR+1

            lda     #0
            sta     OPERAND
@loop
            lda     BEFORE_P
            pha
            lda     BEFORE_A
            plp
            adc     OPERAND
            php
            ldy     #0
            sta     (PTR),y             ; Store ADC A result
            pla
            ldy     #1
            sta     (PTR),y             ; Store ADC P resu;t

            lda     BEFORE_P
            pha
            lda     BEFORE_A
            plp
            sbc     OPERAND
            php
            ldy     #2
            sta     (PTR),y             ; Store SBC A result
            pla
            ldy     #3
            sta     (PTR),y             ; Store SBC P result
            inc     OPERAND
            bne     @loop

            rts
