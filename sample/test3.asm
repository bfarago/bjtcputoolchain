; This is a test asm file
; for BJT CPU.
; The codes below are only for a collection of the
; possible usecases and syntax.
const1  equ 0xc00	; 12 bit wide constant
const2	equ 3		; 4bit wide constant
const3  equ const1	; symbols must be declared before use in const syntax
const4  equ const1+1; can be expression (a+b)
const5  equ const1+const2
const6  equ const2*2	
const7  equ const2<<1	    ; shift up by one bit
const8  equ const1>>const2	; shift down

nible0  equ const1@0		; shift down n*4 bits
nible1  equ const1@1		; like label@hi and @lo,
nible2  equ const1@2		; but here we have 3 suffix

; const2 	equ const2+1	; override is an error
; maybe this can be useful, we should decide...
; Error, because of the replacement is not possible,
; only works in equ format, with predefined operand.

_n1     EQU 0xF000/const1	; const with can be 32 bits
                            ; but it is truncated later
;where comments starts, the rest of the line skipped.
;all of the reserved words are case insensitive, but
;variable names are sensitive to case.
;valid symbole name's first character must be [a-z_.]
;format. And the rest of the positions are: [a-z_.0-9]

;numbers starts with numbers. two hex format is valid:
hex1	equ 0f1h ; with h at the end
hex2	equ 0xf1 ; with 0x prefix
;binary numbers have b suffix
bin1	equ 01011b; this is a 4bit wide const

;reserved word 'global' is valid, but do not use!
;sections are used only at lst output to display format
;of the content at this moment.
section .code  ; the first section is .code by defult
;so it is not necessary to be specified.
;But other section names, which doesn't content 'code'
;will produce a data format section.

resetvec:	     ; label :  here the ':' is important,
; it means the resetvec get the actual address like this:
otherway EQU $	 ; this is the same, but in const syntax.

;two nibbles long instruction
		mvi a,const4*2 ; strict format "mvi" [one space] "a,"
;actually the parser accepts only in this format

;four nibbles long instructions
		lda const1		; load from const address
		sta const1+1	; store to const address
		ad0 const1	; load memory and add to acc
		ad1	const1	; load memory and add +1 to acc
		adc const1	; load memory and add carry to acc
		nand const1 ; not and
		nor	const1	; not or
		rrm const1	; rotate right
		jnz $+28	;jump when not acc is not zero
label2:	jz label2	;jump when zero
		jm label2	;when acc bit3 is set, negative
		jp	label2	;when acc bit3 is not set
		jc	label2	;jump when carry bit was set
		jnc	label2	;jump when no carry
		jmp label2	;unconditional jump
label3:		; if label declared before usage, it works
;like a const, simple... pass1

		jmp subrutin	;when symbol is unknown,
;in pass1 it gets 0 value, and resolved later in pass2.
;only + operation can be used in this way but
; jmp (label+0xfff) means label-1
ret1:	lda temp	;pop acc
		rrm data+1	;load memory rotate right
		lda const_8
		jmp subrutin
ret2:	lda temp
halt:	jmp halt

subrutin:
		sta temp	;reserve acc
		jm case1	;when acc>7
case0:	jmp ret1	;when acc bit3 is zero
case1:	jmp ret2

section .data	;change to data section from here

temp: db 0	; store one byte data to actual position
data: 0, 1	; this format also works
decimals:  0,1,2,3,4,5,6,7,8,9 ; one number
greater: 0ah, 0xb, 12, 13 ;only numbers possible here
  db 7+7	; expression can be used only in db format
  db 30/2	; like this
0x10	; only the low order 4 bits will be stored

long_len equ $	; get actual position first 4 bits 
long1:	db long_len>>8	;then store  highest 4 bits
		db long_len>>4  ;middle 4 bits
		db long_len		;low 4 bits
long2:	db long_len@2	;the same but other format
		db long_len@1
		db long_len@0

org $+16>>4<<4			; alignment to 16
v_text:		; lets define a 16 character long ascii string, 
 "GAME OVER |_[   "
 ; it stores 16 lows and 16 highs nibbles.
 ; first n nibbles are the low, then n nibbles for high part.
 "0123456789"		; this defines 10*2 nibbles all highs are 0.
 ; and 
 "HELLO"	;5*2 datas: 2E55810111
 "HELLO WORLD!" ; 12*2 data: 2E558F08B5D8 10111F211102
 ;actual abc / ascii conversion works for this letters:
 "0123456789ABCDEF"
 "GHIJKLMNOPQRSTUV"
 "WXYZ.,:;!?><=+-/" ; signs... But actually, there is quote possible:
 "\()#@ü$ |_[┌┐┘└l" ; the \ is a normal char. Therefore no \" possible.
 "╚{}o"	;last known graphical characters.
 " " ; space is ff. If you quote all letters, then low and high 
 ;or alternate value syntax
 "H""E""L""O" ; nibbles are alternating: 20e0515181
 ;one nibble syntax
 "HE"+	; only high nibbles: 10
 "HE"-	; only low nibbles: 1e
 ;special character
 ":THIS:":	; replace : to " character.
 

        end
