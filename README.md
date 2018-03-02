# bjtcputoolchain
Toolchain for a BJT cpu

ASSEMBLER
---------
It use flex as scanner. That called from an internal "grammar", which implement simple symbol table, relocation table (two pass compiliation).

[Assembler syntax](doc/assembler.md)

[CPU Instruction set](doc/instruction_set.md)

Usage:
------
  asmb.exe filename.asm
  
  or type file.asm |asmb.exe
  
  writes a binary file:  a.out and a.lst
  
  See test.bat in sample director.

EMULATOR
--------
It read a binary output file (got from asmb), and simulates the CPU internals, and bus signals.

