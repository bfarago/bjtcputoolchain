# bjtcputoolchain
Toolchain for a BJT cpu

TODOs
-----

I just collected several things here, to get some progress... :)

Missing infos from Hw/Sw interface:
-----------------------------------

Exact list of the keypad matrix codes, including the arrow peripherial.

Downloader design.

Half-implemented, therefore limited Sw features:
------------------------------------------------

Expressions can only handle one operator +, when at least one symbol is not yet
definied. So, pass 2 compilation can only add one precalculated constant, and one symbol.

Symbole operations are syntactically correct (/, >>, <<, *), but only plus sign will work correctly. (same as previous)

There is the plan, to handle kind of extras like: multiple asm files, with external and globl symbols, memory ranges for sections, etc. But not yet fully
implemented. 

There is the plan to add macro to lexical and grammatical parser. Not yet implemented.

There is several plans to implement call/jsr and ret/rst, but not yet implemented.

There is a plan to add screen designer User Interface to IDEb project. Because of the ascii table is not trivial in an asm file.

Actually there is an implementation of the include file handling in asmb, ideb and intermediate .dbg file. Line number handling is problematic.

Line numbers could be wrong (+-1 or more difference).  I did not start to figure out why, but looks like the root-cause should be in the asmb and lexer. IDEb is adapted to the failure now.

There is the plan to design and implement "player" projects. Probably it will run on Windows, uCs, FPGAs.


