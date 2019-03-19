;test jsr and ret
	section code
reset:
	mvi a,0  ; do something
	
	#include "inc_jsr.asm"
	jmp subroutine1	; call subroutine1
	
	mvi a,1  ; do something

	#include "inc_jsr.asm"
	jmp subroutine2 ; call subroutine2
	
	jmp reset
	
subroutine1: 
	sta work1
	mvi a,2
	jmp return

subroutine2:
	sta work2
	mvi a,3
	; jmp return ; or
	#include "inc_ret.asm"

	section data
work1: db 0
work2: db 0

