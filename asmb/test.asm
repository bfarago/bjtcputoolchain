; This is a test asm file
; for BJT CPU.


org 0x000
; rhgi8_ equ 0x102+resetcounter

resetvector:
                lda resetcounter ; 0x1100
                inc
                sta resetcounter
                sub resetcounter2
                jz  start
                ; clear
                lda 0x00
                sta other_variable
                
start:          lda other_variable
                inc
                sta other_variable
				lda externalvar
                jmp start

org 0x100
resetcounter:   db 0x00
resetcounter2:  db 0xff
other_variable: db 0xa5

                end
