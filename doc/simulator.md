# bjtcputoolchain
Toolchain for a BJT cpu

SIMULATOR
---------
Command line tool to simulate the BJT CPU.
It reads the binary output of the assembler.

![simulator](ss1.png?raw=true "simulator")

Columns:

BUS
---
Addr: Address bus 3*4=12 bits wide

D: Data bus 4 bits wide

S: State (Read or Write)

CPU
---

S: State F:Fetch, L:Load, D:Decode, S:Store

PC: Program Counter register 12bits wide

I: Instruction register 4bits wide

Work: Work register 12bits wide

ALU
---

AlU: Aritmethical and Logical Unit register

Flag: ALU flags: C:Carry, M:Minus, Z:Zero

Perif: Specific part of the memory , which is used as periferial ports.
