# bjtcputoolchain
Toolchain for a BJT cpu

Uart:
-----
The uart peripherial in the sw was written from scratch. The original hw implementation is unknown. Actual sw implementation can transmit and receive one byte.
The load and store bus cycles can reach two different uart registers. Reading of the register gets Rx data, while writing the register
will set Tx data. Unreaded Rx data register will be overwritten by the bus when new data arrives. Writing of Tx data low, will trigger
shift register to send out the buffer. The actual simulator can trace the tx traffic, but doesn't stimulate the Rx direction yet.