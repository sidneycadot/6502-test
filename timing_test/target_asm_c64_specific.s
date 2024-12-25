
                .export _pre_big_measurement_block_hook
                .export _post_big_measurement_block_hook
                .export _set_irq_vector_address
                .export _zp_address_is_safe_for_read
                .export _zp_address_is_safe_for_write
                .export _measure_cycles

                .include "c64.inc"

                .data

save_border_color:
                .res 1

                .code

_pre_big_measurement_block_hook:

                lda     VIC_BORDERCOLOR
                sta     save_border_color

                sei                     ; Disable interrupts.

                lda     #$2f            ; Prepare to disable of the screen.

_raster_02f_wait:

                cmp     VIC_HLINE
                bne     _raster_02f_wait

                bit     VIC_CTRL1       ; Avoid raster $12f.
                bmi     _raster_02f_wait

                lda     #0              ; Disable the screen.
                sta     VIC_CTRL1

_raster_031_wait:

                lda     #$31            ; Ensure the screen has been disabled.
                cmp     VIC_HLINE
                bne     _raster_031_wait

                lda     #1              ; Disable CIA#1 timer A underflow interrupt
                sta     CIA1_ICR

                lda     CIA1_ICR        ; Clear CIA#1 interrupt flags.

                cli                     ; Enable interrupts.

                rts

_post_big_measurement_block_hook:

                lda     #$1b            ; Restore the screen.
                sta     VIC_CTRL1

                lda     #129            ; Enable CIA#1 timer A underflow interrupt
                sta     CIA1_ICR

                lda     save_border_color
                sta     VIC_BORDERCOLOR

                rts

_set_irq_vector_address:

                ; Set BRKVec, the IRQ handler address for software IRQs caused by the BRK instructions,
                ; to AX, and return the old address in AX.

                ldy     BRKVec                          ; old vector, LSB
                sta     BRKVec                          ; new vector, LSB
                lda     BRKVec+1                        ; old vector, MSB
                stx     BRKVec+1                        ; new vector, MSB
                tax
                tya
                rts

_zp_address_is_safe_for_read:
_zp_address_is_safe_for_write:

                ; On the C64, any ZP address other than 0 and 1 is safe.

                cmp     #1      ; iff A > 1, the carry will be set.
                lda     #0
                adc     #0      ; A is now equal to (address > 1).
                ldx     #0
                rts

_measure_cycles:

                ; We get a 6502 subroutine address as a pointer in X (MSB) and A (LSB).
                ; This subroutine will he called later.

                tay    ; Preserve A (LSB).

                ; The 6502 "rts" instruction will return to the address found on the stack increased by 1.

                ; First, push return address from the testcode.
                ; Once we're in the test code and reach an RTS, we should return back to the '@return_from_testcode' address.

                lda     #>(@return_from_testcode - 1)
                pha
                lda     #<(@return_from_testcode - 1)
                pha

                ; Next, push the address to jump to the testcode upon RTS.

                sec
                tya
                sbc     #<1
                tay
                txa
                sbc     #>1
                pha
                tya
                pha

                ; Stop timer B of CIA#1.

                lda     #0
                sta     CIA1_CRB

                ; Pre-load the 16-bit counter of timer B to 0x00ff.

                sta     CIA1_TB+1
                lda     #255
                sta     CIA1_TB

                ; Reload and start timer B in one shot mode.

                lda     #$19
                sta     CIA1_CRB

@execute_testcode_subroutine:

                rts     ; Jump into the 'testcode' subroutine, and return to @return_from_testcode when we reach the RTS that ends it.

@return_from_testcode:

                ; 12 cycles of overhead (2 x RTS).

                ; Stop timer B of CIA#1.

                lda     #0          ; 2 cycles
                sta     CIA1_CRB    ; 4 cycles

                ; Retrieve timer B counter value (LSB).

                lda     CIA1_TB     ; 4 cycles

                ; The timer counts down from 255; compute the number of CPU cycles counted.

                eor     #255

                ; Subtract 17 cycles to only measure the start of the test-code subroutine to its RTS.

                sec
                sbc     #17

                ldx     #0
                rts
