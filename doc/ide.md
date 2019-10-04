# bjtcputoolchain
Toolchain for a BJT cpu

IDE
---
The latest windows executable added to reposityory in bin folder. The exe is a statically linked MFC application, hopefuly run
on your windows 8 and 10. There is no linux port of the gui. Command line executables like asmb, simb and playb could compile on linux,
in example raspberry. The same asmb commandline tool is used by the ideb to compile actual project. It is started as background piped process.
The simulator use not only the binary file, but the debug output of the assembler. This one contains all of the necessary debug informations,
like connections between binary and source lines, symbols, etc.

The actual implementation have a syntax highlighted viewer for asm files, scroll, cursor move are implemented, but no editor feature is done.
The asm viewer can show dis-assembled dump on the left side of the module, which shows heat-map of the earlier bus operations. Which helps on
understanding how the code was running in the simulator.

![ide screenshot](/doc/sside2.jpg?raw=true "ide and descriptions")

The jump type of instruction also collected by the simulator, so the displayed call trace shows where the program counter was jumped in
the previous microsecounds. This part of the monitor view is interactive, curve can be zoomed by a mouse scroll.

HOW TO START?
-------------

Starting the ideb.exe from bin folder, it starts without projects. Therefore first we get a new file wizard, where new project should
be choosen. That means, we creates an ideb project file in memory. The project file stores two important parameter: main source file, and
assembler tool path. These are filled by default. In case you have a different asm file, then [...] button at the right will open a file browser
dialog to choose source file. The main assembly file is important, because the same path and name will be used for the generated files as well.
The new file wizard can be opened by File -> New menu later.

![new project wizard](/doc/idenewproject.png?raw=true "ide new project")

After we have an earlier project, we can close the new project wizard, and open an existing ideb project file.

Ideb project file parameters can be shown and can be changed from properties module:

![project properties](/doc/ideprojprop.png?raw=true "ide project properties")

In case of the project is clean, there was no compilation before, then build the project with tool button or menu Build -> Compile.
In case of the project was built manually in the background while the ide was open, then ide may not recognize the new debug file.
Therefore "Reload" tool button or Debug -> Reload will load the binary and debug file to the simulator. Memory dump must show the new
RAM content in this case.

When a project contains all the necessary files, it is loaded, then simulation can starts by pressing one of the debug buttons: Play or Step.

![run step](/doc/idebrunstep.png?raw=true "ide run and step")

When simulator is in running state, all of the cpu related display elements will be updated periodically by "Refresh rate". Of course, multiple
cpu states has been simulated on this time of period. See statistical display of the monitor.

Double click on asm source file will open the file in the simulator, as a syntax highlighted source view module.

![file view](/doc/idefileclass.png?raw=true "ide file view")

Double click on left class view element, will navigate for the particular line in the asm view.

Dragging of the module tab header, we can organise the layout.

![drag view](/doc/idedragviews.png?raw=true "drag view")

Double clicking on the left white column of the asm view, we can add and remove breakpoints if there are instruction for that
specific line. The numbers of breakpoints are not limited, every memory location could have a breakpoint. When PC steps into a
location of breakpoint, the simulated cpu gets into the paused state. 

At paused CPU state we can resume running operation by pressing RUN button, or we can debug in steps. The state step operation will broke down
one instruction's execution for as many cpu clock, as needed for the related instruction. This one is good for understanding the CPU
internal operations, because the bus and cpu states are displayed in this way. While instruction step was pressed, it will exectute a
complete operation chain, namely one instruction will be executed only. (this one is the usual "step debug" operation when
code level debugging is on progress.) Good idea, to add multiple breakpoints rather than step by step debuging...

![brakepoints](/doc/idebreak.png?raw=true "ide brakepoints")

Also very informative, to watch red heat-map lines where the cpu is running. We can figure out which conditional jump was made by cpu
earlier, which part of the code was skipped.

Blue lines means the memory location was written earlier. Notice: code is running from RAM, so operands can be rewritten by code.

Green lines means the cpu was read the memory location.

![trace](/doc/idetrace.png?raw=true "ide timeline trace")

The View menu -> Timeline switch turns on the monitor's timeline trace, where execution history can be seen.



