; This is a test asm file
; for BJT CPU.
org 0x100
resetcounter:	db 0x00
resetcounter2:	db 0x00
other_variable:	db 0x00

org 0x000
resetvector:
				lda resetcounter
				inc
				sta resetcounter
				sub resetcounter2
				jz  start
				; clear
				lda 0x00
				sta other_variable
				
start:			lda other_variable
				inc
				sta other_variable
				jmp start
				end

