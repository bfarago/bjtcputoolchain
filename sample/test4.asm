section data
label:
 ; 0123456789abcdef
 "GAME OVER       "d
section code

 alma equ 11
lda 0x123>>4<<4+1
lda 1+2+3+4
lda 3*2+1
org  $+16>>4<<4
aligned:
	jmp 0{}
mvi a, nemvan
mvi a, 0x100



section data
org  $+16>>4<<4
aligned2:
	1, 2, 3
