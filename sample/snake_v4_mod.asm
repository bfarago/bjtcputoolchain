out1	equ 3072
out2	equ 3073
beep	equ 3074
arr		equ 3072
rnd		equ 3073
key0	equ 3074
key1	equ 3075
cord_x	equ 3076
cord_y	equ 3077
ch_h	equ 3078
ch_l	equ 3079
timing	equ 5

section .code
	;call clrscr
	; include this file multiple times, as many call you need
	sta $+27		; keep the acc
	mvi a, $+22
	sta clrscr_ret0
	mvi a, $+16>>4
	sta clrscr_ret0+1
	mvi a, $+10>>8
	sta clrscr_ret0+2	; (+10)
	jmp clrscr
	mvi a,0			; keep the acc (+6)
					; 4 for jmp after the include

	mvi a,7
	sta cord_x
	sta cord_y
	sta old_x
	sta old_y	
	mvi a,4
	sta ch_h
	mvi a,2
	sta len
	sta out2
	mvi a,0
	sta len2
	sta out1
	;call food
	; include this file multiple times, as many call you need
	sta $+27		; keep the acc
	mvi a, $+22@0
	sta food_ret0
	mvi a, $+16@1
	sta food_ret0+1
	mvi a, $+10@2
	sta food_ret0+2	; (+10)
	jmp food
	mvi a,0			; keep the acc (+6)
					; 4 for jmp after the include

cold_start:	lda arr
	sta dir
	ad0 db_1	;is it F?
	jz cold_start
	jmp put_snake
	
warm_start:	lda dir
	ad1 db_4	;is it 11?
	jz up
	
	lda dir
	ad1 db_1	;is it 14?
	jz down
	
	lda dir
	ad1 db_2	;is it 13?
	jz left
	
	lda dir
	ad1 db_8	;is it 7?
	jz right
	
up:	lda db_Y
	sta old_y
	ad0 db_F	;decrement Y
	sta db_Y
	jmp rest
	
down:	lda db_Y
	sta old_y
	ad0 db_1	;increment Y
	sta db_Y
	jmp rest
	
left:	lda db_X
	sta old_x
	ad0 db_F	;decrement X
	sta db_X
	jmp rest

right:	lda db_X
	sta old_x
	ad0 db_1	;increment X
	sta db_X
	jmp rest

;calculating the rest of the snake
rest:	lda len
	sta temp
	lda len2
	sta temp2
xpos:	lda db_X
	lda xpos+1
	ad0 db_1	;increment X array
	sta xpos+1
	sta xseg+1
	sta x2seg+1
	lda xpos+2
	adc db_0
	sta xpos+2
	sta xseg+2
	sta x2seg+2
	lda xpos+3
	adc db_0
	sta xpos+3
	sta xseg+3
	sta x2seg+3
xseg:	lda db_X	;to be overwrite
	sta act_x	;save the actual X coordinate of the part to be overwrite
	lda old_x	;overwrite with the old coordinate of n-1. part
x2seg:	sta db_X	;to be overwrite
	lda act_x
	sta old_x	;the new old_x
;decrementing the lenght
	lda temp2
	nor db_0	;inverting
	sta temp2
	lda temp
	nor db_0
	ad0 db_1	;decrement by 1
	sta temp
	lda temp2
	adc db_0	;adding the carry
	nor db_0
	sta temp2
	lda temp
	nor db_0
	sta temp
	nor temp2	;temp NOR temp2
	nor db_0	;inverting the result
	jnz xpos
x3seg:	lda db_X	;restore the address
	lda x3seg+1
	sta xpos+1
	lda x3seg+2
	sta xpos+2
	lda x3seg+3
	sta xpos+3
	lda len
	sta temp
	lda len2
	sta temp2
ypos:	lda db_Y
	lda ypos+1
	ad0 db_1	;increment Y array
	sta ypos+1
	sta yseg+1
	sta y2seg+1
	lda ypos+2
	adc db_0
	sta ypos+2
	sta yseg+2
	sta y2seg+2
	lda ypos+3
	adc db_0
	sta ypos+3
	sta yseg+3
	sta y2seg+3
yseg:	lda db_X	;to be overwrite
	sta act_y	;save the actual Y coordinate of the part to be overwrite
	lda old_y	;overwrite with the old coordinate of n-1. part 
y2seg:	sta db_X	;to be overwrite
	lda act_y
	sta old_y	;the new old_y
	;decrementing the lenght
	lda temp2
	nor db_0	;inverting
	sta temp2
	lda temp
	nor db_0
	ad0 db_1	;decrement by 1
	sta temp
	lda temp2
	adc db_0	;adding the carry
	nor db_0
	sta temp2
	lda temp
	nor db_0
	sta temp
	nor temp2	;temp NOR temp2
	nor db_0	;inverting the result
	jnz ypos
y3seg:	lda db_Y	;restore the address
	lda y3seg+1
	sta ypos+1
	lda y3seg+2
	sta ypos+2
	lda y3seg+3
	sta ypos+3
	lda db_X	;restore old_x, old_y variables to the first element (the head of the snake)
	sta old_x
	lda db_Y
	sta old_y
	jmp put_snake

; put the snake to the screen using the db_X and db_Y arrays
put_snake:	lda len
	sta temp
	lda len2
	sta temp2
pxseg:	lda db_X
	sta cord_x
pyseg:	lda db_Y
	sta cord_y
	lda temp
	nor temp2	;temp NOR temp2
	nor db_0	;inverting the result
	jnz p2seg
	mvi a,5
	sta ch_l	;delete element after length
	jmp p3seg
p2seg:	mvi a,13
	sta ch_l
	;incrementing the address after pxseg by 1
	lda pxseg+1
	ad0 db_1
	sta pxseg+1
	lda pxseg+2
	adc db_0
	sta pxseg+2
	lda pxseg+3
	adc db_0
	sta pxseg+3
	;incrementing the address after pyseg by 1
	lda pyseg+1
	ad0 db_1
	sta pyseg+1
	lda pyseg+2
	adc db_0
	sta pyseg+2
	lda pyseg+3
	adc db_0
	sta pyseg+3
	;decrementing the lenght
	lda temp2
	nor db_0	;inverting
	sta temp2
	lda temp
	nor db_0
	ad0 db_1	;decrement by 1
	sta temp
	lda temp2
	adc db_0	;adding the carry
	nor db_0
	sta temp2
	lda temp
	nor db_0
	sta temp
	jmp pxseg
	;restore addresses
p3seg:	lda db_X
	lda p3seg+1
	sta pxseg+1
	lda p3seg+2
	sta pxseg+2
	lda p3seg+3
	sta pxseg+3
p4seg:	lda db_Y
	lda p4seg+1
	sta pyseg+1
	lda p4seg+2
	sta pyseg+2
	lda p4seg+3
	sta pyseg+3	

;selfcollosion detection
selfcol:	lda len
	sta temp
	lda len2
	sta temp2
	;increment index X
cstart:	lda cxseg+1
	ad0 db_1
	sta cxseg+1
	lda cxseg+2
	adc db_0
	sta cxseg+2
	lda cxseg+3
	adc db_0
	sta cxseg+3
	;increment index Y
	lda cyseg+1
	ad0 db_1
	sta cyseg+1
	lda cyseg+2
	adc db_0
	sta cyseg+2
	lda cyseg+3
	adc db_0
	sta cyseg+3
	;decrementing the lenght
	lda temp2
	nor db_0	;inverting
	sta temp2
	lda temp
	nor db_0
	ad0 db_1	;decrement by 1
	sta temp
	lda temp2
	adc db_0	;adding the carry
	nor db_0
	sta temp2
	lda temp
	nor db_0
	sta temp
	ad0 db_F	;-1 (investigate until lenght-2
	nor temp2	;temp-1 NOR temp2
	nor db_0	;inverting the result
	jz cx3seg
cxseg:	lda db_X	;next element to investigate
	sta g2ate
	nor db_0	;invert it
	nand db_X	;half of XOR
	sta gate
	lda db_X
	nor db_0
	nand g2ate
	nand gate
	jz cyseg
	jmp cstart	;if no X mach found
cyseg:	lda db_Y	;next element to investigate
	sta g2ate
	nor db_0	;invert it
	nand db_Y	;half of XOR
	sta gate
	lda db_Y
	nor db_0
	nand g2ate
	nand gate
	jz found
	jmp cstart ;if no Y match found
cx3seg:	lda db_X	;restore addresses
	lda cx3seg+1
	sta cxseg+1
	lda cx3seg+2
	sta cxseg+2
	lda cx3seg+3
	sta cxseg+3
cy3seg:	lda db_Y	;restore addresses
	lda cy3seg+1
	sta cyseg+1
	lda cy3seg+2
	sta cyseg+2
	lda cy3seg+3
	sta cyseg+3
	jmp read
found:	mvi a,1
	sta beep
	;call delay
	; include this file multiple times, as many call you need
	sta $+27		; keep the acc
	mvi a, $+22@0
	sta delay_ret0
	mvi a, $+16@1
	sta delay_ret0+1
	mvi a, $+10@2
	sta delay_ret0+2	; (+10)
	jmp delay
	mvi a,0			; keep the acc (+6)
					; 4 for jmp after the include
	mvi a,0
	sta beep
	jmp cx3seg
	
;putting food to the screen (food_x, food_y)
food:	mvi a,5
	sta ch_h
	lda rnd
	ad1 rnd
	ad1 db_X
	sta food_x
	sta cord_x
	lda rnd
	ad1 rnd
	ad1 db_Y
	sta food_y
	sta cord_y
	mvi a,0
	sta ch_l
	mvi a,4
	sta ch_h
	; return from subrutin "food"
	food_ret0 equ  $+1
	jmp 0    ; this address wil be replaced by work_ret0..2 

;is the head of the snake on the food?
onfood:	lda db_X	;the head
	nor db_0	;invert it
	nand food_x	;half of XOR
	sta gate
	lda food_x
	nor db_0
	nand db_X
	nand gate
	jnz oseg
	lda db_Y	;the head
	nor db_0	;invert it
	nand food_y	;half of XOR
	sta gate
	lda food_y
	nor db_0
	nand db_Y
	nand gate
	jnz oseg
	;if the head is on the food
	mvi a,2
	sta beep
	lda len
	ad0 db_1
	sta len
	sta out2
	lda len2
	adc db_0
	sta len2
	sta out1
	mvi a,0
	sta beep
	;call food
	; include this file multiple times, as many call you need
	sta $+27		; keep the acc
	mvi a, $+24&22
	sta food_ret0
	mvi a, $+16>>4&15
	sta food_ret0+1
	mvi a, $+10>>8&15
	sta food_ret0+2	; (+10)
	jmp food
	mvi a,0			; keep the acc (+6)
					; 4 for jmp after the include
	
oseg:	mvi a,0
	;ret
	onfood_ret0 equ  $+1
	jmp 0    ; this address wil be replaced by work_ret0..2 
	
read:	;call delay
	; include this file multiple times, as many call you need
	sta $+27		; keep the acc
	mvi a, $+22@0
	sta delay_ret0
	mvi a, $+16@1
	sta delay_ret0+1
	mvi a, $+10@2
	sta delay_ret0+2	; (+10)
	jmp delay
	mvi a,0			; keep the acc (+6)
					; 4 for jmp after the include
					
	;call onfood	
	; include this file multiple times, as many call you need
	sta $+27		; keep the acc
	mvi a, $+22@0
	sta onfood_ret0
	mvi a, $+16@1
	sta onfood_ret0+1
	mvi a, $+10@2
	sta onfood_ret0+2	; (+10)
	jmp onfood
	mvi a,0			; keep the acc (+6)
					; 4 for jmp after the include
	lda arr
	sta temp
	ad0 db_1	;is it F
	jz warm_start
	lda dir		;actual direction
	ad1 db_4	;is it 11?
	jz reup
	lda dir
	ad1 db_1	;is it 14?
	jz redown
	lda dir
	ad1 db_2	;is it 13?
	jz releft
	lda dir
	ad1 db_8	;is it 7?
	jz reright
reup:	lda temp	;new direction
	ad1 db_1	;is it 14, down?
	jz warm_start
	lda temp
	sta dir
	jmp warm_start
redown:	lda temp	;new direction
	ad1 db_4	;is it 11, up?
	jz warm_start
	lda temp
	sta dir
	jmp warm_start	
releft:	lda temp	;new direction
	ad1 db_8	;is it 7, right?
	jz warm_start
	lda temp
	sta dir
	jmp warm_start		
reright:	lda temp	;new direction
	ad1 db_2	;is it 13, left?
	jz warm_start
	lda temp
	sta dir
	jmp warm_start	
	
clrscr:	mvi a,15
	sta ch_h
	mvi a,0
	sta cord_x
l_1:	ad0 db_F
	sta cord_y
	lda cord_x
l_2:	ad0 db_F
	sta cord_x
	sta ch_l
	jnz l_2
	lda cord_y
	jnz l_1
	;ret
	clrscr_ret0 equ  $+1
	jmp 0    ; this address wil be replaced by work_ret0..2 
	
delay:	mvi a,timing
	sta time_a
	sta time_b
	sta time_c
de0:	mvi a,timing
de1:	ad0 db_F
	jnz de1
	lda time_a
	ad0 db_F
	sta time_a
	jnz de0
	lda time_b
	ad0 db_F
	sta time_b
	jnz de0
	lda time_c
	ad0 db_F
	sta time_c
	jnz de0
	; ret
	delay_ret0 equ  $+1
	jmp 0    ; this address wil be replaced by work_ret0..2 
halt:	jmp halt

section .data
db_0:	0
db_1:	1
db_2:	2
db_3:	3
db_4:	4
db_5:	5
db_6:	6
db_7:	7
db_8:	8
db_9:	9
db_A:	10
db_B:	11
db_C:	12
db_D:	13
db_E:	14
db_F:	15

db_X:	7,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
db_Y:	7,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

g2ate:	0
gate: 0
len2: 0
temp2: 0
temp:0
dir:0
len:0
food_x:0
food_y:0
old_x:0
old_y:0
act_x:0
act_y:0
time_a	: 0
time_b	: 0
time_c	: 0


	end