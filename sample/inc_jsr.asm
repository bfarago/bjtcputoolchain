	; include this file multiple times, as many call you need
	sta $+23		; keep the acc
	mvi a, $+24@0
	sta work_ret0
	mvi a, $+18@1
	sta work_ret0+1
	mvi a, $+12@2
	sta work_ret0+2	; (+10)
	mvi a,0			; keep the acc (+6)
					; 4 for jmp after the include
					