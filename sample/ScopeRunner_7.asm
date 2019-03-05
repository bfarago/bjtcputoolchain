; peripherals
arr		equ 3072
rnd		equ 3073
key0	equ 3074
key1	equ 3075
cord_x	equ 3076
cord_y	equ 3077
ch_h	equ 3078
ch_l	equ 3079
;architecture properties
addr_h	equ 3	;place of high nibble of address in instruction
addr_m	equ 2	;place of middle nibble of address in instruction
addr_l	equ 1	;place of low nibble of address in instruction
;abstaction to help using VRAM with coordinates
cx	equ 1
cy	equ 2

section .code
;------------------------------------------------------------------------------------------------------------
;first instruction at 0x000
	mvi a,0
	sta v_gstart+1
v_start:	mvi a,0
	sta cord_x	;init cord_x
	sta cord_y	;init cord_y
	;reset target position modification delta
	sta v_tpmd+1
	;reset jump counter
	sta v_jmpctr+1
	;reset jump timer
	sta v_jmptimer+1
	;reset field timer
	mvi a,15
	sta v_fieldtimer+1
	;reset speed increase timer
	sta v_sptimerL+1
	sta v_sptimerH+1
	;reset last column sprite pointer
	sta p_colsprite+cx
	;reset field speed
	sta v_fieldperiod+1
	;reset player position
	mvi a,9
	sta p_plsprite1+cy
	sta p_plsprite2+cy
	sta p_plsprite3+cy
	sta p_plsprite4+cy
	sta p_delpl2+cy
	sta p_delpl1+cy
	;reset block position counter
	mvi a,2
	sta v_blockctr+1
	mvi a,3
	sta ch_h
	;do not reset v_gstart

;------------------------------------------------------------------------------------------------------------
;main loop
;copy VRAM to video
; the order of accessing peripheral registers is important: modifying ch_l triggers drawing a character (based on cord_x, cord_y, ch_h and ch_l itself)
p_draw:	lda db_vram0
	;character decoding: characters are described by two nibbles, but VRAM contains only one nibble for each character
	jnz l_draw1
	mvi a,15	;if VRAM element is 0, draw space instead of '\'
	sta ch_h
	;if VRAM element is not 0, draw the graphic character
l_draw1:	sta ch_l
	;restore is made unconditional to make decoding reset-robust
	mvi a,3	;restore ch_h to the default value, which represents the part of character set with graphic characters
	sta ch_h
	;increment X and set
	lda p_draw+cx
	ad0 db_1
	sta cord_x	;restoring VRAM is reset-robust only with this order of sta-s
	sta p_draw+cx
	jnc p_draw	;increment Y only when all X in a row were handled
	;increment Y and set
	mvi a,0	;this instruction has no effect on carry flag
	adc p_draw+cy
	sta cord_y
	sta p_draw+cy
	jnc p_draw
	;at the end the manipulated addresses overflow to 0, so we don't need to reinit them
v_gstart:	mvi a,0	;by default it's 0 meaning that the game is not started. It will be changed with self-programming when the game starts
	jnz v_sptimerL
;reset key peripheral right before using it
	lda key0
	lda key1
;waiting for a key to start game (except arrow keys, because it can be still/again pressed during/after game over)
l_waitkey:	lda key0
	ad0 key1	;if any key is pressed, the sum won't be 0, or if the sum overflows to 0, the carry won't be 0
	jnz l_actgame
	jc l_actgame
	jmp l_waitkey	;at address 217 (0x0D9)
;activate game with self-programming: modify values at v_gstart and st to change path of jump instructions
l_actgame:	mvi a,1
	sta v_gstart+1
;restore VRAM
p_rest_dvram:	lda db_dvram0
p_rest_vram:	sta db_vram0
	lda p_rest_dvram+cx
	ad0 db_1
	sta p_rest_vram+cx	;restoring VRAM is reset-robust only with this order of sta-s
	sta p_rest_dvram+cx
	jnc p_rest_dvram
	mvi a,0	;this instruction has no effect on carry flag
	adc p_rest_dvram+cy
	sta p_rest_vram+cy
	sta p_rest_dvram+cy
	jnc p_rest_dvram
	;at the end the manipulated addresses overflow to 0, so we don't need to reinit them
	jmp v_start	;restoring VRAM is needed after game over. It's unconditional to make things easier
	;TODO: jumps over the db_* section

section .data
;------------------------------------------------------------------------------------------------------------	
;constants
db_0:	0
db_1:	1	;can be used for increment: ad0 db_1
db_2:	2
db_3:	3
db_4:	4
db_5:	5
db_6:	6
db_7:	7
db_8:	8
db_9:	9
db_A:	0xA
db_B:	0xB
db_C:	0xC
db_D:	0xD
db_E:	0xE
db_F:	0xF	;can be used for decrement: ad0 db_F
	
;VRAM default content (0x100)
;first 4 nibbles are the jmp v_start instruction
;this is used for storing sprites as well
;		0 1 2 3 4 5 6 7 8 9 A B C D E F
db_dvram0:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvram1:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvram2:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvram3:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvram4:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvram5:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvram6:	0xC,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xB,0x7
db_dvram7:	0x8,0x0,0x0,0x0,0x0,0x1,0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xA,0x0
db_dvram8:	0x8,0x0,0x0,0x0,0x0,0xC,0xB,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xA,0x0
db_dvram9:	0x8,0x0,0x0,0x0,0x0,0xD,0xE,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xA,0x0
db_dvramA:	0x0,0x7,0x7,0x7,0x7,0x7,0x7,0x7,0x7,0x7,0x7,0x7,0x7,0x7,0x0,0x0
db_dvramB:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvramC:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvramD:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvramE:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_dvramF:	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
;VRAM: character addresses are 0x2YX (in order of address nibbles in instructions: XY2)
; When addressing VRAM, high nibble of the address is not manipulated by self programming. So even if something goes wrong, it will mess up only the VRAM.
;		0 1 2 3 4 5 6 7 8 9 A B C D E F
db_vram0:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vram1:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vram2:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vram3:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vram4:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vram5:	0,0x0,0x9,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vram6:	0,0x8,0x9,0x0,0x0,0x0,0x0,0x0,0x8,0x7,0x0,0x0,0x0,0x0,0x0,0x0
db_vram7:	0,0x0,0x8,0x8,0x7,0x1,0x2,0x6,0x8,0x7,0x0,0x0,0x0,0x0,0x0,0x0
db_vram8:	0,0x0,0x7,0x0,0x7,0x0,0x0,0xA,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0
db_vram9:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x8,0x7,0x0,0x0,0x0,0x0,0x0,0x0
db_vramA:	0,0xB,0x8,0x8,0x0,0x3,0x0,0x3,0x8,0x7,0x0,0xB,0x0,0x0,0x0,0x0
db_vramB:	0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x7,0x0,0x0,0x0,0x0,0x0,0x0
db_vramC:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vramD:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vramE:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
db_vramF:	0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
;jump timer constants
;       0 1 2 3 4 5 6 7 8 9 a b c d e f
db_jt:	0,0x0,0xF,0xA,0x7,0x5,0x4,0x2,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0

section .code
;------------------------------------------------------------------------------------------------------------
;active game logic
;decrement speed increase timer
v_sptimerL:	mvi a, 15
	ad0 db_F
	sta v_sptimerL+1
	jc l_delplayer
v_sptimerH:	mvi a,15
	ad0 db_F
	sta v_sptimerH+1
	jc l_delplayer
;restart speed increase timer
	mvi a,15
	sta v_sptimerL+1
	sta v_sptimerH+1
;increase field speed (decrease timer period)
	lda v_fieldperiod+1
	ad0 db_F
	jc l_setfspeed
	mvi a,0	;minimum is 0
l_setfspeed:	sta v_fieldperiod+1
;delete player sprite because it should be redrawn because of field movement or jumping/falling
l_delplayer:	mvi a,0
p_delpl1:	sta db_vram9+5
p_delpl2:	sta db_vram9+6
	lda p_delpl1+cy
	ad0 db_F
	sta p_delpl2+cy
	sta p_delpl1+cy
	jc l_delplayer
	;at the end the Y coordinates will be F. They will be corrected after the player sprite re-draw
;decrement field move timer
; this part of the code is also self-programming: it self-contains the timer variables
v_fieldtimer:	mvi a,15
	ad0 db_F
	sta v_fieldtimer+1
	jc l_jump
;move the field left with one pixel
; shift the whole screen to make it future-proof
; VRAM is handled now as a 1-dimensional array to be shifted left
; the last column will be messed up, but it's not a problem because it will be overwritten with the new column content
p_shiftfield:	lda db_vram0+1	;instruction containing the source pointer 00
p_shifttarget:	sta db_vram0	;instruction containing the target pointer FF
	lda p_shiftfield+addr_m
	sta p_shifttarget+addr_m
	lda p_shiftfield+addr_l
	sta p_shifttarget+addr_l
	ad0 db_1
	sta p_shiftfield+addr_l
	jnc p_shiftfield
	mvi a,0
	adc p_shifttarget+addr_m
	sta p_shiftfield+addr_m
	jnc p_shiftfield
	;at the end the p_shifttarget=0x_FF and p_shiftfield=0x_00
;correct last column sprite pointer (if it's E, change to F)
	mvi a,1
	ad1 p_colsprite+cx	;a = 1+sprite_pointer+1
	jnc v_blockctr
	mvi a,15
	sta p_colsprite+cx
;decrement block position counter
; block position counter represents place of the block at the right side of the screen
; one block is 4 pixels wide
v_blockctr:	mvi a,2
	ad0 db_F
	sta v_blockctr+1
	jc p_colsprite
;reset block position counter
	mvi a,3
	sta v_blockctr+1
;generate random number (just 1 bit needed for the block)
v_ran:	mvi a,14
	nor rnd	;xxx0->0001, xxx1->0000
;overwrite last column sprite pointer based on the new random number
	jnz l_setcolsprite
	ad1 p_colsprite+cx	;a=0+sprite_pointer+1 = 2 or 0 because when block position counter is 0, last column sprite can be only 1 or F
	jc p_colsprite
	;if carry: new block is high, prev block was high. So we don't need to draw falling edge as before-last column and we don't need to change the last column sprite pointer
	;if not carry: new block is high, prev block was low. Last column sprite should be rising edge. It will be changed to field high sprite during the next field shift
	mvi a,14
	sta p_colsprite+cx
	jmp p_colsprite
l_setcolsprite:	ad0 p_colsprite+cx	;a=1+sprite_pointer = 2 or 0 because when block position counter is 0, last column sprite can be only 1 or F
	jnc p_colsprite
	mvi a,1
	sta p_colsprite+cx
	;if carry: new block is low, prev block was high. Draw falling edge as before-last column and modify last column sprite pointer to 1 (low field)
	;if not carry: new block is low, prev block was low. So we don't need to draw falling edge as before-last column and we don't need to change the last column sprite pointer
;draw before-last column sprite (copy from default VRAM array to VRAM)
p_beflastcol:	lda db_dvramF
p_blcoltarget:	sta db_vramF+14
	lda p_beflastcol+cy
	ad0 db_F
	sta p_blcoltarget+cy
	sta p_beflastcol+cy
	jc p_beflastcol
	;Y coordinates end up with F
;draw last column sprite
; this instruction contains the column pointer manipulated above
; possible values: 0=falling edge, 1=low field, E=rising edge, F=high field
p_colsprite:	lda db_dvramF+15
p_target:	sta db_vramF+15
	lda p_colsprite+cy
	ad0 db_F
	sta p_target+cy
	sta p_colsprite+cy
	jc p_colsprite
	;Y coordinates end up with F
;restart field timer
; the literal in the instruction will be self-programmed to be smaller and smaller to make the timer shorter and shorter periods to make the field move faster and faster
; reset-robustness: if reset occurs during timer reinit, at least the high nibble is reinited making the first cycle after reset as long as possible
v_fieldperiod:	mvi a,15
	sta v_fieldtimer+1
;check if jump is commanded (so arr is 11). If jump is commanded, store it in v_tpmd
l_jump:	mvi a,5
	ad0 arr
	jnz l_playertop
	lda v_tpmd+1
	ad0 db_1
	jc l_playertop	;if jump is already active, skip jump counter
	;only two jumps are allowed between ground touches
v_jmpctr:	mvi a,0
	ad0 db_E
	jc l_playertop
	ad0 db_3
	sta v_jmpctr+1
	mvi a,15
	sta v_tpmd+1	;store jump command
;check if player is on the top (y=2). In this case make the player fall with modifying target position modification delta to +1
l_playertop:	mvi a,13
	ad0 p_plsprite2+cy
	jc l_gndchk
	mvi a,1
	sta v_tpmd+1
;check stuff relative to the ground
l_gndchk:	lda p_plsprite2+cy
	ad0 db_1
	sta p_gndchk1+cy
	sta p_gndchk2+cy
p_gndchk1:	lda db_vram9+5
p_gndchk2:	ad0 db_vram9+6
	adc db_0	;this will be zero only when both p_gndchk1 and p_gndchk2 are zero
	jz l_gndchk3
	;player is on the ground...
	mvi a,1
	ad0 v_tpmd+1	;1+v_tpmd=2 or 1 or 0(with carry)
	jc v_jmptimer
	;...and it is standing on the ground or falling (so target position modification delta is 0 or +1)
	mvi a,0
	sta v_jmpctr+1	;reset jump counter
	sta v_tpmd+1	;overwrite to standing
	;reset timer
	sta v_jmptimer+1
	jmp v_jmptimer
	;player is not on the ground...
l_gndchk3:	lda v_tpmd+1
	jnz v_jmptimer
	;...and floating
	mvi a,1
	sta v_tpmd+1	;make it fall
;decrement jump timer
v_jmptimer:	mvi a,0
	ad0 db_F
	sta v_jmptimer+1
	jc l_collision
;timer is over
;modify target pointers (p_plsprite2, p_plsprite4) with target position modification delta
; if v_tpmd = 15, jumping is active
; if v_tpmd = 1, faling is active
; if v_tpmd = 0, player is standing
v_tpmd:	mvi a,0
	ad0 p_plsprite2+cy
	sta p_plsprite4+cy
	sta p_plsprite2+cy
;restart jump timer
;new timer value depends on the target position
	lda p_plsprite2+cy	;already modified with the delta
	sta p_newjmpT+addr_l
p_newjmpT:	lda db_jt
	sta v_jmptimer+1
;check collision
l_collision:	lda p_plsprite2+cy	;already modified with the delta
	sta p_collision+cy
	;if char is not 0, v_gstart should be 0
p_collision:	lda db_vram9+6
	jz p_plsprite1
	mvi a,0
	sta v_gstart+1
;draw player sprite
p_plsprite1:	lda db_dvram9+5
p_plsprite2:	sta db_vram9+5
p_plsprite3:	lda db_dvram9+6
p_plsprite4:	sta db_vram9+6
	;decrement source (address of sprite)
	lda p_plsprite1+cy
	ad0 db_F
	sta p_plsprite3+cy
	sta p_plsprite1+cy
	;decrement target (coordinates of VRAM)
	lda p_plsprite2+cy
	ad0 db_F
	sta p_plsprite4+cy
	sta p_plsprite2+cy
	jc p_plsprite1
	;restore target pointers based on final value of source pointers
	;initial_target_pointer = 9 - final_src_pointer + 1 = ... = NOT(final_src_pointer) + 9
	mvi a,0
	nor p_plsprite1+cy
	ad0 db_9
	sta p_plsprite4+cy
	sta p_plsprite2+cy
	;restore pointers at player sprite deletion too
	sta p_delpl2+cy
	sta p_delpl1+cy
	;restore source pointers
	mvi a,9
	sta p_plsprite3+cy
	sta p_plsprite1+cy
	jmp p_draw
	end