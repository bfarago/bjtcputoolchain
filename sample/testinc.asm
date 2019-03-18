; This is a test include asm file
; for BJT CPU.
section .code1
uart_base   equ 0xc00
uart_h  	equ uart_base+0
uart_l  	equ uart_base+1
uart_in0    equ uart_base+2
uart_in1    equ uart_base+3

	jmp $+4
	jmp $+4
	jmp $+4
	jmp $+4
