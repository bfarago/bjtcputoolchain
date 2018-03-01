; This is a test asm file
; for BJT CPU.
perif   equ 0xc00
offs1	equ 3
h_uart  equ perif+0
l_uart  equ perif+1
in0     equ 2+perif
in1     equ perif+offs1
m1_addr EQU 4095
m1      Equ 15

section .code
		mvi a,m1
		sta m1_addr
		jmp delay		;call delay
start:  jmp clear		;call clear
retclr: mvi a,3
        sta h_uart
        ldA db_er
        STa l_uart
        lda db_er+1
        sta l_uart
        lda in0
        sta l_uart
        lda in1
        sta l_uart
        jmp start

clear:  mvi a,0             ;clear rutin
        sta h_uart
        mvi a,12
        sta l_uart
        jmp retclr			;ret
		
delay:  ad0 m1_addr         ;delay rutin
        sta 4093
        mvi a,15
loop3:  ad0 m1_addr
        jnz loop3
        lda 4093
        jnz delay
        jmp start			;ret
halt:   jmp halt

section .data
db_er:  0,1,2,3,4,5,6,7,8,9
db_len: db $-db_er
long_len equ $
long1:	db long_len>>8
		db long_len>>4
		db long_len
long2:	db long_len@2
		db long_len@1
		db long_len@0
        end