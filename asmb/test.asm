; This is a test asm file
; for BJT CPU.
perif   equ 0xc00
h_uart  equ perif+1
l_uart  equ perif+2
in0     equ perif+3
in1     equ perif+4
m1_addr equ 4095
m1      equ 15

		mvi a,m1
		sta m1_addr
		jmp delay
start:  jmp clear
retclr: mvi a,3
        sta h_uart
        lda db_er
        sta l_uart
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
        jmp retclr
		
delay:  ad0 m1_addr         ;delay rutin
        sta 4093
        mvi a,15
loop3:  ad0 m1_addr
        jnz loop3
        lda 4093
        jnz delay
        jmp start
halt:   jmp halt

db_er:  0,1,2,3,4,5,6,7,8,9

        end