; This is a test asm file
; for BJT CPU.
;PERIPHERIALS
periph 		equ 0xc00
periph_scr  equ periph+4
periph_uart equ periph+8

arr			equ periph+0
rnd			equ periph+1
key0		equ periph+2
key1		equ periph+3
scr_x	  	equ periph_scr+0
scr_y  		equ periph_scr+1
scr_h  		equ periph_scr+2
scr_l  		equ periph_scr+3
uart_oh  	equ periph_uart+0
uart_ol  	equ periph_uart+1
uart_ih  	equ periph_uart+2
uart_il  	equ periph_uart+3

;APPLICATION CONSTANTS
c_delay4 equ 0x11-3 ; condition will be checked after the first delay, and 0x10 will produce carry, 3*150ms ~ 500ms


;CODE
section .code
reset:
		mvi a,0
		sta v_y
		sta v_x
		sta v_h
		sta v_l
		sta v_delay1
		sta v_delay2
		sta v_delay3
		
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
		ad0 plus+1
		sta v_x
		jnc loop1 ; still in the same row
		mvi a,1
		ad0 v_y
		sta v_y
		jnc loop1 ;still in the screen
		
		;increment abc low nibble
		lda v_l
		ad0 plus+1
		sta v_l
		jnc start_delay
		
		;increment abc high nibble
		lda v_h
		ad0 plus+1
		sta v_h
		;check, if last available abc char displayed?
		ad0 minus-5	; add n to the actual value, which will produce carry
		jnc start_delay
		mvi a,0 ;restart abc
		sta v_h
		
		
		;end of screen changer part
start_delay:		
		mvi a, c_delay4
		sta v_delay4		;init delay last nibble (outer loop number) to wait n* 2^16 
delay1:
		mvi a,0
		ad0 plus+1			; 2*tick (0x02)
		jnc $-4				; inner loop. Jump back one 4_12 instruction, which is 4 nibble long. 
							; time= 1+16*2 tick = 33 (=2<<4+1)
		v_delay1 equ $+1	; this variable will be on immediate of the next mvi instruction
		mvi a, 0
		ad0 plus+1
		sta v_delay1
		jnc delay1			; loop 1. time = n*(1+16*2 +4)=n*(1+9*4)
							; time= 16+ 9*64 (= (2<<4+5)<<4 )
		v_delay2 equ $+1	; this variable will be on immediate of the next mvi instruction
		mvi a, 0
		ad0 plus+1
		sta v_delay2
		jnc delay1			; loop 2. time = ((2<<4+5)<<4+5)<<4 = 9552
		
		v_delay3 equ $+1	; this variable will be on immediate of the next mvi instruction
		mvi a, 0
		ad0 plus+1
		sta v_delay3
		jnc delay1			; loop 3. time= (((2<<4+5)<<4+5)<<4+5)<<4 =152912  (152ms)
		
		v_delay4 equ $+1	; this variable will be on immediate of the next mvi instruction
		mvi a, 0
		ad0 plus+1
		sta v_delay4
		jnc delay1			; loop 4. time=((((2<<4+5)<<4+5)<<4+5)<<4+5)*n = ~500ms if n==3
		jmp loop1

; DATA SECTION
section .data
plus: 0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf
minus: 0x0 ; use : minus-n and plus+n
end
