
; This is a generic (target independent) wrapper around the target-specific '_measure_cycles' routine.
;
; It adds the functionality of saving the values in certain zero page addresses before calling _measure_cycles
; and restoring them afterwards. This is needed in case the test code to be executed clobbers zero page addresses;
; the operation system (if any) and the C runtime depend on certain zero page addresses to be untouched by
; user code.

                .import _measure_cycles
                .export _measure_cycles_wrapper

                ; These addreses are used by the caller to specify which (if any) zero page addresses
                ; need to be saved.
                ;
                ; These variables are defined in 'timing_test_measurement.c'.

                .import _num_zpage_preserve     ; Number of zero page addresses to be preserved. Should be 0, 1, or 2.
                .import _zpage_preserve         ; Two bytes that can hold the zpage addreses to be preserved.

                .bss

zpage_copy:     .res 2          ; Save any zero-page values (up to two) that need preserving here.

                .code

_measure_cycles_wrapper:

                ; Save pointer to test-code.

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

@done_save:     ; Restore pointer to test-code.

                pla
                tax
                pla

                ; Call the target-specific measurement routine.

                jsr     _measure_cycles

                ; Save result (test-code clock cycles).

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

@done_restore:  ; Restore result (test-code clock cycles).

                pla
                tax
                pla

                ; All done.

                rts
