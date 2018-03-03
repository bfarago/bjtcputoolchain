# bjtcputoolchain
Toolchain for a BJT cpu.
This project was formed on that goal, to have a toolchain for a really existed, hand-made built Bipolar junction Transistors based CPU.
See this short vieo: [Bipolar cpu running at 61Hz](https://www.youtube.com/watch?v=iL6OvX4frJs) 
The processor made out of BC182 (~450 piece), 1N4148 and resistors. The data path is 4-bit, address space is 12-bit. Known max speed is more than 1MHz. There are 3 flags: Sign, Carry and Zero

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

SIMULATOR
--------
It read a binary output file (got from asmb), and simulates the CPU internals, and bus signals.

Usage:
------
  simb.exe filename.out [steps]
  
  steps is optional, specifies the number of steps of the simulation. One step is a complete (fetch-decode-load-store) cycle. Output is textual,
  and contains bus address, data, status signals, then cpu internals: Program counter register, Instruction register, Work register, Alu register,
  and flags: carry, zero, sign. Then periferial status nibbles.
  
  The cpu have specific address space for periferials. Accessing on this range will trigger hard-coded behaviours, so periferial responses will be implemented as well.
 
EMULATOR
--------
Planned...  The goal is to be able to download the binary to real hw, and able to trace, debug the system.
1) as an fpga based device
 
2) as a microcontroller based device.
