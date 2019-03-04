; This is a test asm file
; for BJT CPU.
;PERIPHERIALS
periph_uart  equ 0xc00
uart_oh  	equ periph_uart+0
uart_ol  	equ periph_uart+1
uart_ih  	equ periph_uart+2
uart_il  	equ periph_uart+3

periph_scr   equ 0xd00
scr_x	  	equ periph_scr+0
scr_y  		equ periph_scr+1
scr_h  		equ periph_scr+2
scr_l  		equ periph_scr+3

;APPLICATION CONSTANTS
c_start_offs equ 0x00


;CODE
section .code
reset:
		mvi a,0
		sta spos
		sta spos+1
loop1:	
v_y     equ $+1
		mvi a,0
		sta scr_y
v_x     equ $+1
		mvi a,0
		sta scr_x
v_h     equ $+1
		mvi a,0
		sta scr_h
v_l     equ $+1
		mvi a,0
		sta scr_l
		lda v_x
		ad0 plus1
		sta v_x
		jnc loop1
		mvi a,1
		ad0 v_y
		sta v_y
		jnc loop1
		
		lda v_l
		ad0 plus1
		sta v_l
		jnc loop1
		jmp loop1
		
start_inc:
		mvi a,1
		ad0 v_start_offs+1
		sta v_start_offs+1
		jnz end_inc
		mvi a,1
		ad0 v_start_offs
		sta v_start_offs
end_inc:
		jmp return

		lda spos
		ad0	soff
		sta scr_x
		lda spos
		ad0	soff
		sta scr_x
halt:   
	    jmp halt

; DATA SECTION
section .data
plus1: 1
spos: 0,0
soff: 0,0

v_start_offs:
		db c_start_offs@1
		db c_start_offs@0
end
