# bjtcputoolchain
Toolchain for a BJT cpu

ASSEMBLER
---------
It use flex as scanner. That called from an internal "grammar", which implement simple symbol table, relocation table (two pass compiliation).


![ASM functional diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_seq.puml)

| Format         | Description                                       |
|----------------|---------------------------------------------------|
| **org** address    | processing will continue as the specified address |
| symbol **equ** hexa| this symbol will be equal to the specified value (constant in compilation time) |
| label **:**         | actual address will stored as a symbol/label. Data section also requires a ':', it is not optional.  |
| **db** hexa        | stores a data byte to the actual address |
| mnemonic immediate | opcode and operand, wher immediate is a constant |
| mnemonic symbol    | opcode and operand with an expression|

See the details of [CPU Instruction set](instruction_set.md)

The next diagram shows the grammar main loop, and simplified example of a label (label_target:) or ascii string ("GAMEOVER"+ or "GAMEOVER"-) terminus in lexer and grammar. Ascii charset contains 16*16 letters, but memory is only 4 bits wide, therefore one ascii string should be stored two locations, one of them is for low nibles and another location holds the high nibles. Screen drawing algorithm should get these nibles and send to the vectorgraphic peripherial. In assembly syntax level, we are able to encode these two nibles from the same ascii letters, using the {string}+ or {string}- syntaxes.

![ASM functional diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_asm_fn.puml)

The next grammar function handle string constants, like "SOMETHING"[+-]. Where +: most significant 4 bits, and -: least significant 4 bits. See above.

![parse string diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_string.puml)

The next grammar function process mvi a,{expression} syntax. Where expression is a 4 bits wide literal, so the value will be calculated in compile time at pass #1 or #2.

![Parse op4 12 diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_op4_4.puml)

The next grammar function process all other mnemonics like:  opcode {expression} syntax. Where expression value can be 12 bits wide. Therefore these operators usually address the memory by the operand.

![Parse op4 12 diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_op4_12.puml)


MACRO ASSEMBLER
---------------
The hw doesn't have stack or subrutin (jsr/ret) instruction. But the code is running in RAM (not in Flash) therefore multiple instructions could formulate such complex process like a jump to subrutin and return from subrutin. 

This is a jump to subrutin implementation:

```asm
	mvi a, $+22         ; assembler will calculate the return address using the actual address
	sta subrut_ret0     ; and store the least significant nibble at return
	mvi a, $+16>>4
	sta subrut_ret0+1
	mvi a, $+10>>8      ; assembler will calculate the return address rightmost nibble
	sta subrut_ret0+2	; most significant nibble of the return address stored now
	jmp subrut_label  ; the return address points after the jump instruction 
```

The previous code snippet will calculate the address of the first instruction after the jmp subrutin, due to that will be the return address. The return code consist of one jmp instruction, with 3 address nible.


```asm
	; return from subrut
	subrut_ret0 equ  $+1         ; the symbol subrut_ret0 is memorizing by the assembler here, given the value from the actual address +1
	jmp subrutin_return_address  ; this address will be replaced when call code is executed.
```

When the subrutin reach the previous code, it will jump back to the caller side onto the return address, which was filled at the time of calling the subrutin.
Limitation: no loop is possible in call-graph, because of only one return address belongs to only one caller at a time of subrutin execution.