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
l_outer_loop:
    sta l_read_lower4bit+2
    ad0 l_read_lower4bit+2
    sta l_read_lower4bit+2
    ad0 v_plus+1
    sta l_read_higher4bit+2
    mvi a,0
l_printonerow:
    sta l_read_higher4bit+1
    sta l_read_lower4bit+1
    sta cord_x
l_read_higher4bit:	
    lda v_text+16
    sta ch_h
l_read_lower4bit:	
    lda v_text
    sta ch_l
    lda l_read_higher4bit+1
    ad0 v_plus+1
    jnz l_printonerow
    
    rrm l_read_lower4bit+2 
    ad0 v_plus+2
    jm halt ;if greater than 8
    rrm l_read_lower4bit+2
    ad0 v_plus+1 
    sta cord_y
    jmp l_outer_loop
halt:
    jmp halt
section data
v_plus:
 "0123456789ABCDEF"-
 org 0x100 ;aligned
v_text:
 "HELLO WORLD!    "
 "HAPPY NEW YEAR! "
 "   _  _  _  _   "
 "  | [| [| [ _[  "
 "   / | [ /   [  "
 "  |_ |_[|_  _[  "
 "                "
end