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