; This is a test asm file
; for BJT CPU.
section code
cord_x	equ 3076
cord_y	equ 3077
ch_h	equ 3078
ch_l	equ 3079

start:
l_print:
	mvi a,0
	sta cord_y
l_gameover_loop3:
	sta l_gameover_loop2+2
	ad0 l_gameover_loop2+2
	sta l_gameover_loop2+2
	ad0 v_plus+1
	sta l_gameover_loop1+2
	mvi a,0
l_gameover_loop0:
	sta l_gameover_loop1+1
	sta l_gameover_loop2+1
	sta cord_x
l_gameover_loop1:	
	lda v_text+16
	sta ch_h
l_gameover_loop2:	
	lda v_text
	sta ch_l
	lda l_gameover_loop1+1
	ad0 v_plus+1
	jnz l_gameover_loop0
	
	lda l_gameover_loop2+1
	rrm v_plus+1
	
	ad0 v_plus+1
	sta cord_y
	ad0 v_plus+1
	jmp l_gameover_loop3
	
halt:
	jmp halt

section data
v_plus:
 "0123456789ABCDEF"-
 
 org $+256>>8<<8
v_text:
 "HELLO WORLD! _  "
 "GAME OVER   |_| "

 
  end