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



Examples, description:
----------------------

|asm example | Description                                        |
|------------|----------------------------------------------------|
| mvi a,0x1  | Move from immediate to Accumulator                 |
| sta 0x123  | Store Acc. to memory (to imm addr). |
| lda 0x123  | Load from memory data (on imm addr) to Acc. |
| ad0 0x123  | Add Acc. to loaded memory data (from imm addr) and keep result in Acc. |
| ad1 0x123  | Add Acc.+1 to loaded memory data (from imm addr) and keep result in Acc. |
| adc 0x123  | Add Acc.+Carry to loaded memory data (from imm addr) and keep result in Acc. |
| nand 0x123 | Nand Acc. and loaded data (from imm addr) and keep result in Acc. |  
| nor 0x123  | Nor Acc. and loaded data (from imm addr) and keep result in Acc. |
| rrm 0x123  | Rotate right. acc= mem[literal] / 2  |
| jmp 0x123  | Unconditional jump to address, the value of 12bit literal. |
| jz 0x123   | Jump if zero  (all 4 bits in acc) |
| jnz 0x123  | Jump if non-zero  (one of the acc bits is high) |
| jm 0x123   | Jump if minus  ( if bit 3 is high ) |
| jp 0x123   | Jump if positive ( if bit 3 is low ) |
  
![CPU functional diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.github.com/bfarago/bjtcputoolchain/master/doc/cpu_fn.puml)

  etc...
  See the test files in sample folder.
  

