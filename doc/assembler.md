# bjtcputoolchain
Toolchain for a BJT cpu

ASSEMBLER
---------
It use flex as scanner. That called from an internal "grammar", which implement simple symbol table, relocation table (two pass compiliation).

org address     ; processing will continue as the specified address

symbol equ hexa ; this symbol will be equal to the specified value (constant in compilation time)

label:          ; actual address will stored as a symbol/label. Data section also requires a ':', it is not optional.

db hexa         ; sores a data byte to the actual address

mnemonic immediate

mnemonic symbol

mnemonic

[CPU Instruction set](instruction_set.md)

The next diagram shows the grammar main loop, and simplified example of a label (label_target:) or ascii string ("GAMEOVER"+ or "GAMEOVER"-) terminus in lexer and grammar. Ascii charset contains 16*16 letters, but memory is only 4 bits wide, therefore one ascii string should be stored two locations, one of them is for low nibles and another location holds the high nibles. Screen drawing algorithm should get these nibles and send to the vectorgraphic peripherial. In assembly syntax level, we are able to encode these two nibles from the same ascii letters, using the {string}+ or {string}- syntaxes.

![ASM functional diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_asm_fn.puml)

The next grammar function handle string constants, like "SOMETHING"[+-]. Where +: most significant 4 bits, and -: least significant 4 bits. See above.

![ASM functional diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_string.puml)

The next grammar function process mvi a,{expression} syntax. Where expression is a 4 bits wide literal, so the value will be calculated in compile time at pass #1 or #2.

![ASM functional diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_op4_4.puml)

The next grammar function process all other mnemonics like:  opcode {expression} syntax. Where expression value can be 12 bits wide. Therefore these operators usually address the memory by the operand.

![ASM functional diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/parse_op4_12.puml)
