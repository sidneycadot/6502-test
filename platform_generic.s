
                .import _measure_cycles
                .import _zp_address_is_safe
                .export _measure_cycles_zp_safe

                .bss

SAVE_ZEROPAGE:  .res    256

                .code

_measure_cycles_zp_safe:

                ;

                pha                                     ; Save A and X (test-code address pointer),
                txa
                pha

                ldx     #0                              ; Copy zero page to SAVE_ZEROPAGE.
@saveloop:      txa
                pha
                jsr     _zp_address_is_safe
                tay
                pla
                tax
                tya
                beq     @skip_save

                lda     0,x
                sta     SAVE_ZEROPAGE,x

@skip_save:     inx
                bne     @saveloop

                pla                                     ; Restore A and X (test-code address pointer).
                tax
                pla

                jsr     _measure_cycles

                pha                                     ; Save the A and X return value (cycle count).
                txa
                pha

                ldx     #0                              ; Copy SAVE_ZEROPAGE back to zero page.
@restore_loop:  txa
                pha
                jsr     _zp_address_is_safe
                tay
                pla
                tax
                tya
                beq     @skip_restore

                lda     SAVE_ZEROPAGE,x
                sta     0,x

@skip_restore:  inx
                bne     @restore_loop

                pla                                     ; Restore A and X resturn value (cycle count).
                tax
                pla

                rts
