
                .importzp sp
                .import incsp4
                .export _sha_absy

                .code

                ; void __fastcall__ sha_absx (unsigned int abs, unsigned char ra, unsigned char rx, unsigned char ry)

                .proc   _sha_absy

                sta     ldy_opc+1       ; On entry, the A register holds the "ry" argument.

                ldy     #3              ; Load MSB of "abs" argument.
                lda     (sp),y
                sta     sha_absy_opc+2

                dey                     ; Load LSB of "abs" argument.
                lda     (sp),y
                sta     sha_absy_opc+1
                dey

                lda     (sp),y          ; Load the "ra" argument.
                sta     lda_opc+1
                dey                     ; Load the "rx" argument.
                lda     (sp),y
                sta     ldx_opc+1

lda_opc:        lda     #0              ; Immediate value will be patched.
ldx_opc:        ldx     #0              ; Immediate value will be patched.
ldy_opc:        ldy     #0              ; Immediate value will be patched.

                ; Execute opcode.

sha_absy_opc:   .byte   $9f             ; Illegal opcode: SHA abs,y (0x9f)
                .word   0
                ; Tear down the stack frame and return.

                jmp     incsp4

                .endproc
