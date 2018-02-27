# bjtcputoolchain
Toolchain for a BJT cpu


ASSEMBLER
---------
It use flex as scanner. That calls internal "grammar", which implements simple symbol table, relocation table (two pass compiliation).

org address     ; processing will continue as the specified address

symbol equ hexa ; this symbol will be equal to the specified value (constant in compilation time)

label:          ; actual address will stored as a symbol/label. Data section also requires a ':', it is not optional.

db hexa         ; sores a data byte to the actual address

mnemonic immediate

mnemonic symbol

mnemonic

Usage:
------
  asmb.exe filename.asm
  or type file.asm |asmb.exe
  writes a binary file:  a.out

Instruction set:
----------------
OP_4_12:

[Byte 0     ] [Byte 1     ]

[7654] [3210] [7654] [3210] 

OPCODE IMMEDIATE_12BIT

OP_4_4:

[Byte 0     ]

[7654] [3210]

OPCODE IMMEDIATE_4BIT

inc Increment acc.

Examples, description:
----------------------

  mfi 0x123 Move from immediate to Accumulator
  
  lda 0x123 Load from memory data (on imm addr) to Acc.
  
  lda 0x123 Store Acc. to memory (to imm addr).
  
  add 0x123 Add Acc. to loaded memory data (from imm addr) and keep result in Acc.
  
  sub 0x123 Sub Acc. from loaded memory data (from imm addr) and keep result in Acc.
  
  jmp 0x123
  
  jz 0x123
  
  jnz 0x123

  etc...
