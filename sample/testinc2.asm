; This is a test implementation of push and pop.

; for BJT CPU.
section .code_push_and_pop
	 
l_push:	; start of push, which is the default behaviour
	sta v_stack					; store accumulator on top of the stack.
	v_stack_ptr0 equ l_push + 1	; first immediate is the lowest address nibble

	; if mode was pop then exit 
	v_stack_mode equ $+1		; condition for jump
	mvi a,0
	jnz l_pop_exit
	
	;increment stack pointer (address for the next push)
	mvi a,1
	ad0 v_stack_ptr0
	sta v_stack_ptr0
	jnc l_push_exit
	mvi a,1
	ad0 v_stack_ptr0+1
	sta v_stack_ptr0+1
	jnc l_push_exit
	mvi a,1
	ad0 v_stack_ptr0+2
	sta v_stack_ptr0+2
	jmp l_push_exit
	
l_pop: ; start of pop
	mvi a, 2 			; opcode of the lda
	sta l_push  		; change the instruction code to lda
	sta v_stack_mode	; change the condition up there
	; now it is not push but pop, do it...
	; decrement stack pointer
	mvi a,0xf
	ad0 v_stack_ptr0
	sta v_stack_ptr0
	jnz l_push
	mvi a,0xf
	adc v_stack_ptr0+1
	sta v_stack_ptr0+1
	jnz l_push
	mvi a,0xf
	adc v_stack_ptr0+2
	sta v_stack_ptr0+2
	jmp l_push
	
l_pop_exit:
	mvi a, 1 			; opcode of the sta
	sta l_push  		; change the instruction code to lda
	mvi a, 0 			; zero is the false
	sta v_stack_mode	; change the condition up there
l_push_exit:
; end