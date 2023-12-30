# bjtcputoolchain
Toolchain for a BJT cpu

Download:
---------

The actual hw implementation contains a replacable SRAM modul, and a stand-alone downloader.
The downloader has its own uart, and hex format textual ascii string can be sent to write the SRAM content.
The first two character is 12 and the last two character is 30. Between this framing blocks, the hexdump block contains all of the memory content.

Further ideas:
--------------

IDEA1. Using FTDI chip:

FTDI chips can work in BitBang mode, when 8 pins can be driven by 8 data bits. (Not in uart mode!)

```c
#define PIN_TX  0x01
#define PIX_RX  0x02
#define PIN_RTS 0x04
#define PIN_CTS 0x08
#define PIN_DTR 0x10
#define PIN_DSR 0x20
#define PIN_DCD 0x40
#define PIN_RI  0x80
```

The idea is here to drive bus from an usb ftdi module, to write the SRAM from the developers PC. For this goal, we need:
4 bits data, at least mimimum one bit to control bus, one bit to cpu state.
Functions:

| pin | function                             |
|-----|--------------------------------------|
| 1   | State. 0: Running/disconnected. 1: hold cpu in reset while programming. Rising edge resets the address counter? |
| 4   | drive data bus, when writing to SRAM |
| 1   | Increment address and/or             |
| 1   | Store/write bus cycle |

IDEA2:

Use an usb or wifi/bluetooth capable micro, write a downloader first, then a bus tracer for debug purposes...

IDEA3: (realized by hw)

use a microcontroller (ATMEGA8) and connect on UART of a PC.
asci numbers, 00-0F. 
12 start
30 end.
