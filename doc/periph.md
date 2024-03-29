# bjtcputoolchain
Toolchain for a BJT cpu

PERIPHERIALS
---------------

Not only the processor but also the necessary input / output peripherals have been built. Therefore, our simulator has a part that
can replace them. The following paragraph describes the actual state of the simulator, while the complete Hardware-Software Interface
will be specified.

Memory map:
----------------

| Start | End   | Description                       |
|-------|-------|-----------------------------------|
| 0x000 | 0xbff | Operative memory (3k Static RAM ) |
| 0xc00 | 0xc00 | Cursor move Arrows                |
| 0xc01 | 0xc01 | Random generator (4 bits wide)    |
| 0xc02 | 0xc03 | Keypad matrix (2x4 bits wide)     |
| 0xc04 | 0xc07 | Screen registers                  |
| 0xc08 | 0xc09 | UART                              |
| 0xc0a | 0xcff | unused peripherial address space  |
| 0xd00 | 0xfff | Operative memory (768bytes Static RAM) |

In case of the most significant nibble is 0xc, then periferial address space is active. The complete 4k address fits with a static RAM.
The low page is 3k long, while the high ram page is 1024-256 bytes long.

Registers description:
----------------------

| Address| Bits | R/W | Init | Description                   |
|--------|------|-----|------|-------------------------------|
| 0xc00  | 0    | R   | 1    | 0: Cursor down arrow pressed  |
| 0xc00  | 1    | R   | 1    | 0: Cursor right arrow pressed |
| 0xc00  | 2    | R   | 1    | 0: Cursor up arrow pressed    |
| 0xc00  | 3    | R   | 1    | 0: Cursor left arrow pressed  |
| 0xc01  | 0-3  | R   | xxxx | Random number 0-15            |
| 0xc02  | 0-3  | R   | 0000 | Keypad row                    |
| 0xc02  | 0-1  | W   | 0000 | Beep 1kHz                    |
| 0xc03  | 0-3  | R   | 0000 | Keypad column                 |
| 0xc04  | 0-3  | W   | 0000 | Screen pos X                  |
| 0xc05  | 0-3  | W   | 0000 | Screen pos Y                  |
| 0xc06  | 0-3  | W   | 0000 | Character High.               |
| 0xc07  | 0-3  | W   | 0000 | Character Low, update.        |
| 0xc08  | 0-3  | R/W | 0000 | Uart data High (obsolate)     |
| 0xc09  | 0-3  | R/W | 0000 | Uart data Low  (obsolate)     |

Cursor arrows:
--------------
Four individual buttons (joystick) are connected to the registers bits. The register is read only.

Random number:
--------------
True random generator is built in analog component level design, the register produce a new number for every bus read operation.

Keypad matrix:
--------------
CPU have a read-only access for two registers. It contains the pressed key code, recognised by keypad row and column. The keypad
matrix scanned by the hw. The registers should be polled by Sw periodically.

Beep output:
--------------
There is an audible frequency oscillator, and it is gated by the regiter values. The lsb bit 0 is the loudest, and bit 1 is a bit quieter volume of the sound. 

Video card:
-----------
The video card could drive an oscilloscope in XY mode. It is clocked from system clock, and refresh the screen autonomously from its own
video memory. The character generator is based on a vector graphic principle. The vector graphics are stored its own EPROM. 
CPU can update the display screen memory. Writing 4 data bits to Character Low register, will replace one screen memory location,
indexed by Screen X and Y registers, and holds Character High and Low.

Uart:
-----
The latest hw doesn't contains uart capability. The previous SW simulation is documented here
[here](periph_uart.md)

Downloader (plan)
----------------------
The actual and planned [downloader](periph_download.md) are documented, but the hw implementation have no debug or download interfaces specificly.

