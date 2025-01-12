
; This is a convenience routine to wait for a number of clock cycles indicated in
; the A (LSB) and X (MSB) registers. The minimum number of cycles that can be accommodated
; is 29. The cycle count includes the JSR into this routine and the RTS back to the calling
; routine, but not any loads to the A/X registers beforehand.
;
; *IMPORTANT* The branches in this code should not cross pages, since that costs an extra
; clock cycle, throwing off the clock cycle count. So be careful to check where this routine
; ends up in memory.
;
; This routine is copied from https://github.com/bbbradsmith/6502vdelay.

                .export _vdelay

                .align 64

_vdelay:        cpx     #0
                bne     @8
                sbc     #33
                bcc     @5
@1:             sbc     #5
                bcs     @1
                adc     #3
                bcc     @2
                lsr
                beq     @3
@2:             lsr
@3:             bcs     @4
@4:             rts
@5:             adc     #3
                bcc     @6
                beq     @6
                lsr
@6:             bne     @7
@7:             rts
@8:             sbc     #5
                bcs     @8
                sbc     #5
                dex
                bne     @8
                sbc     #34
                bcs     @1
