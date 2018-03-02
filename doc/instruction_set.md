# bjtcputoolchain
Toolchain for a BJT cpu

INSTRUCTION SET
---------------

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

  mvi a,0x1 Move from immediate to Accumulator
  
  lda 0x123  Load from memory data (on imm addr) to Acc.
  
  sta 0x123  Store Acc. to memory (to imm addr).
  
  ad0 0x123  Add Acc. to loaded memory data (from imm addr) and keep result in Acc.
  
  ad1 0x123  Add Acc.+1 to loaded memory data (from imm addr) and keep result in Acc.
  
  adc 0x123  Add Acc.+Carry to loaded memory data (from imm addr) and keep result in Acc.
  
  nor 0x123  Nor Acc. and loaded data (from imm addr) and keep result in Acc.
  
  nand 0x123 Nand Acc. and loaded data (from imm addr) and keep result in Acc.
  
  jmp 0x123
  
  jz 0x123
  
  jnz 0x123

  etc...
  See the test files in sample folder.
  

