
        .include "sim65.inc"

        .import     _vdelay
        .importzp   tmp1, tmp2

        .export     _time_vdelay

        .code

        .proc _time_vdelay

        ; Time the VDELAY routine, for the number of cycles specified in A/X.
        ;
        ; The VDELAY function is verified by sampling a clock cycle counter before and after
        ; execution. The difference, minus 18 clock cycles to account for overhead,
        ; should be equal to the number of clock cycles requested.

        ; On entry, A/X contain the number of clock cycles we want to wait.
        ; On exit, the actual number of clock cycles as measured is returned in A/X.

        ldy     #COUNTER_SELECT_CLOCKCYCLE_COUNTER      ; Select the latched clock cycle counter.
        sty     peripheral_counter_select               ;
        sty     peripheral_counter_latch                ; Latch clock cycle count before vdelay execution.

        ldy     peripheral_counter_value + 0            ; Store timestamp before vdelay execution.
        sty     tmp1                                    ;
        ldy     peripheral_counter_value + 1            ;
        sty     tmp2                                    ;

        jsr     _vdelay                                 ; Execute the VDELAY routine.

        sta     peripheral_counter_latch                ; Latch clock cycle count AFTER vdelay.

        sec                                             ; Calculate duration = (timestamp after vdelay) - (timestamp before vdelay).
        lda     peripheral_counter_value + 0            ;
        sbc     tmp1                                    ;
        tay                                             ; Transfer LSB of result to Y, before subtracting overhead.
        lda     peripheral_counter_value + 1            ;
        sbc     tmp2                                    ;
        tax                                             ; Transfer MSB of result to X.

        sec                                             ; Subtract 18 cycles of overhead from duration, and return
        tya                                             ; the resulting VDELAY clock cycle count in A/X.
        sbc     #18                                     ;
        bcs     @done                                   ;
        dex                                             ;

@done:  rts                                             ; All done.

        .endproc
