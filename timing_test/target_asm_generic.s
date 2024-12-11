
        .export _measure_cycles_wrapper
        .import _measure_cycles

        .import _num_zpage_preserve
        .import _zpage_preserve

                .bss

zpage_copy:     .res 2          ; Save any zero-page values that need preserving here.

                .code

_measure_cycles_wrapper:

                pha
                txa
                pha

                ; Save zero-page addresses that need saving.

                ldy     #0
@save_loop:     cpy     _num_zpage_preserve
                beq     @done_save
                ldx     _zpage_preserve,y
                lda     0,x
                sta     zpage_copy,y
                iny
                bne     @save_loop

@done_save:     pla
                tax
                pla

                jsr     _measure_cycles

                pha
                txa
                pha

                ; Restore zero-page addresses that need restoring.

                ldy     #0
@restore_loop:  cpy     _num_zpage_preserve
                beq     @done_restore
                ldx     _zpage_preserve,y
                lda     zpage_copy,y
                sta     0,x
                iny
                bne     @restore_loop

@done_restore:  pla
                tax
                pla

                rts
