
ApiGroup        := $ff00
ApiFunction     := $ff01
ApiError        := $ff02
ApiStatus       := $ff03
ApiParam0       := $ff04
ApiParam1       := $ff05
ApiParam2       := $ff06
ApiParam3       := $ff07
ApiParam4       := $ff08
ApiParam5       := $ff09
ApiParam6       := $ff0a
ApiParam7       := $ff0b

PTR = $CD
FILE_CHANNEL = 3

                .import __RAM_START__, __RAM_LAST__

                .segment "HEADER"

                .byte   3, "NEO"                        ; Magic constant.
                .byte   0, 0                            ; Minimum major/minor version required to work.
                .word   start                           ; Execute address.
                .byte   0                               ; Control bits.
                .word   __RAM_START__                   ; Load address.
                .word   (__RAM_LAST__ - __RAM_START__)  ; Size to load, in bytes.
                .byte   0                               ; String

                .data

TABLE_SIZE = 1024

P_VALUE:        .res    1       ; Value transferred to P before the ADC/SBC  (bits 4 and 5 are ignored).
A_VALUE:        .res    1       ; Value transferred to A before the ADC/SBC.
OPERAND:        .res    1
TABLE:          .res    TABLE_SIZE ; We need 256 * 4 bytes to store results: 256 operand values * {ADC A/P, SBC A/P}.

                .code
start:

PRINT_MSG_START:        ; Print start message to console.

                lda     #0
                sta     OFFSET
@loop:          ldy     OFFSET
                lda     MSG_START,y      ; Load string character
                beq     @done
                sta     ApiParam0

                lda     #6          ; Function 6: Write Character
                sta     ApiFunction
                lda     #2          ; Group 2: Console
                sta     ApiGroup
@wait:          lda     ApiGroup    ; Wait for response.
                bne     @wait

                inc     OFFSET
                jmp     @loop
@done:

OPEN:           ; Open channel for write.

                lda     #4          ; Function 4: File Open
                sta     ApiFunction
                lda     #FILE_CHANNEL
                sta     ApiParam0
                lda     #<FILENAME
                sta     ApiParam1
                lda     #>FILENAME
                sta     ApiParam2
                lda     #3          ; Create/truncate mode.
                sta     ApiParam3

                lda     #3          ; Group 3: File I/O
                sta     ApiGroup
@wait:          lda     ApiGroup    ; Wait for response.
                bne     @wait

                ; Start of A, P loop

                lda     #0
                sta     P_VALUE
                sta     A_VALUE

P_LOOP:         ; Check if we want to test the current value of P_VALUE.

                lda     P_VALUE
                and     #$f6
                cmp     #$32
                bne     SKIP_P_VALUE

A_LOOP:

                ; Print block message to console.

                lda     #0
                sta     OFFSET
@loop:          ldy     OFFSET
                lda     MSG_BLOCK,y      ; Load string character
                beq     @done
                sta     ApiParam0

                lda     #6          ; Function 6: Write Character
                sta     ApiFunction
                lda     #2          ; Group 2: Console
                sta     ApiGroup
@wait:          lda     ApiGroup    ; Wait for response.
                bne     @wait

                inc     OFFSET
                jmp     @loop
@done:

                ; Perform the test for current A, P values.

                jsr     TEST_ADC_SBC

WRITE_TABLE:    ; Write table binary data to channel

                lda     #FILE_CHANNEL
                sta     ApiParam0
                lda     #<TABLE
                sta     ApiParam1
                lda     #>TABLE
                sta     ApiParam2
                lda     #<TABLE_SIZE
                sta     ApiParam3
                lda     #>TABLE_SIZE
                sta     ApiParam4

                lda     #9          ; Function 9: File Write
                sta     ApiFunction
                lda     #3          ; Group 3: File I/O
                sta     ApiGroup
@wait:          lda     ApiGroup    ; Wait for response.
                bne     @wait

                inc     A_VALUE
                bne     A_LOOP

SKIP_P_VALUE:   inc     P_VALUE
                bne     P_LOOP

CLOSE:          ; Close channel.

                lda     #FILE_CHANNEL
                sta     ApiParam0

                lda     #5          ; Function 5: File Close
                sta     ApiFunction
                lda     #3          ; Group 3: File I/O
                sta     ApiGroup
@wait:          lda     ApiGroup    ; Wait for response.
                bne     @wait

PRINT_MSG2:     ; Print end message.

                lda     #0
                sta     OFFSET
@loop:          ldy     OFFSET
                lda     MSG_END,y      ; Load string character
                beq     @done
                sta     ApiParam0

                lda     #6          ; Function 6: Write Character
                sta     ApiFunction
                lda     #2          ; Group 2: Console
                sta     ApiGroup
@wait:          lda     ApiGroup    ; Wait for response.
                bne     @wait

                inc     OFFSET
                jmp     @loop
@done:

DONE:           ; All Done.

                rts

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

                .data

OFFSET:         .res 1

                .rodata

FILENAME:       .byte 9,"adc_sbc_65c02.dat"
MSG_START:      .byte " Start of program!", 13, 0
MSG_END:        .byte " End of program, bye!", 13, 0
MSG_BLOCK:      .byte " Writing 1 KB block ...", 13, 0
