# bjtcputoolchain
Toolchain for a BJT cpu.
This project was formed on that goal, to have a toolchain for a really existed, hand-made built Bipolar junction Transistors based CPU.
See this short vieo: [Bipolar cpu running at 61Hz](https://www.youtube.com/watch?v=iL6OvX4frJs) 
The processor made out of BC182 (~450 piece), 1N4148 and resistors. The data path is 4-bit, address space is 12-bit. Known max speed is more than 1MHz. There are 3 flags: Sign, Carry and Zero

There is also a video card for this CPU. It has two (X,Y) analog outputs, which can be connected to an ordinary oscilloscope to show the screen. The videocard has its own character generator, based on lists of vector graphic steps. Keypad matrix and joystick inputs also implemented.

IDE
---
This is a Windows MFC application, under development. It simulates the BJT CPU, UART, Video card right now. Asm editor and compiler will
be added later. Notepad++ custom syntax highlight config also added to the project.
![ide screenshot](doc/sside1.png?raw=true "ide")
The software shown here was made by one of my colleague Denes Suhayda. It was presented running on the original cpu earlier, now running on the simulator.

ASSEMBLER
---------
It use flex as scanner. That called from an internal "grammar", which implement simple symbol table, relocation table (two pass compiliation).

[Assembler syntax](doc/assembler.md)

[CPU Instruction set](doc/instruction_set.md)

Usage:
------
  asmb.exe filename.asm -l -f
  
  or as pipe: type file.asm |asmb.exe -o a -b a.out -l -f
  
  writes a binary file as *.bin by default or as a.out if -b a.out switch override the default bin output. The -l switch for *.lst and -f switch generate *.coe *.v files.
  
  help: asmb -h
  
  See test.bat in sample directory. Run tests with "test" command, or "test clean" will remove the output files, whenever you run it.

SIMULATOR
--------
It read a binary output file (got from asmb), and simulates the CPU internals, and bus signals.
[See more infos...](doc/simulator.md)

Usage:
------
  simb.exe filename.out [steps]
  
  steps is optional, specifies the number of steps of the simulation. One step is a complete (fetch-decode-load-store) cycle. Output is textual,
  and contains bus address, data, status signals, then cpu internals: Program counter register, Instruction register, Work register, Alu register,
  and flags: carry, zero, sign. Then periferial status nibbles.
  
  The cpu have specific address space for periferials. Accessing on this range will trigger hard-coded behaviours, so periferial responses will be implemented as well.
![simulator](doc/ss1.png?raw=true "simulator")
 
EMULATOR
--------
Planned...  The goal is to be able to download the binary to real hw, and able to trace, debug the system.
1) as an fpga based device
![fpga model](doc/se1.png?raw=true "emulator")
 
2) as a microcontroller based device.
