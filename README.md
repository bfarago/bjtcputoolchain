# bjtcputoolchain
Toolchain for a BJT cpu.
This project was formed on that goal, to have a toolchain for a really existed, hand-made built Bipolar junction Transistors based CPU.
See this short vieos: [Bipolar cpu running at 61Hz](https://www.youtube.com/watch?v=iL6OvX4frJs) ,
[Scope runner](https://www.youtube.com/shorts/DjAvME9jMiA) game running on the hardware.
The processor made out of BC182 (~450 piece), 1N4148 and resistors. The data path is 4-bit, address space is 12-bit. Known max speed is more than 1MHz. There are 3 flags: Sign, Carry and Zero

There is also a video card for this CPU. It has two (X,Y) analog outputs, which can be connected to an ordinary oscilloscope to show the screen. The videocard has its own character generator, based on lists of vector graphic steps. Keypad matrix and joystick inputs also implemented. See [details of the peripherals](doc/periph.md)

IDE
---
This is a Windows MFC application, it is under development. It simulates the BJT CPU, and i/o peripherals like UART, Video card, keypad, joystick. Asm editor part is read only at this moment, and compiler is running externally. To see more detailed description, read [ide documentation](doc/ide.md)

![ide screenshot](doc/sside1.png?raw=true "ide")

The following screenshot also contains some descriptions, we can see how the actual Program Counter shown numerically, and asm view modul also shows by coloring the source line of the actual CPU positions. To analize a longer period of runtime, the ide stores a history for the bus states of every memory cell. This so called heat-map, displayed as colored lines, where blue is for write, green for read, and red for execution cpu-bus states. Jump instructions also triggers the simulator to store locations to a history, this helps to understand the sw behaviour in time domain.  

![ide screenshot](doc/sside2.jpg?raw=true "ide and descriptions")

Furthermore a Notepad++ custom syntax highlight config also added to the project.

We can see a running game on the picture of the simulator. This asm software above was made by one of my colleague Denes Suhayda. It was presented running on the original cpu earlier, now running on the simulator as well. This is a simple side-scrolling game, players can jump to keep the little guy alive, while the scrolling impulses are aproaching faster and faster.

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

3) raspberry

4) a Mac variant is also compilable by xcode or CLI toolchain. Now it is tested on M2 MacBooc.

On progress... Actually the video card simulator is under development. Two PWMs are used for x/y output. The following screenshot shows how the real rpi pins was measured by Analog Discovery2 card, when the playb ran on raspberry...
![raspberry measurement](doc/ssrpi1.png?raw=true "simulator1")
And bitmap mode graphics is also ported to raspberry:
![raspberry screenshot](doc/ssrpi2.jpg?raw=true "simulator2")



