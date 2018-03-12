`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: MYND-ideal
// Engineer: Barna Farago
// 
// Create Date: 04.03.2018 15:06:24
// Design Name: systen bridge
// Module Name: sys
// Project Name: BJTCPU emulator 
// Target Devices: Xilinx
// Tool Versions: 
// Description: 
// bram DATA bus actually 32 bit wide, while cpu's DATA bus is 4 bit wide. The goal of this system interface/bridge to
// realize a virtual 4bit wide memory bus for CPU, based on the system's 32 bit wide memory. To achive this goal, we are
// implementing a simple read-write cache on 32 bits which consits of 4 * bytes = 8 nibbles.
// When cpu read from outside block memory, the sys bridge will read 8 nibbles in one step (if it was not already in the cache).
// In case of a cache hit, no memory read occures (in a future release).
// When cpu writes a nibble:
// 1) in case of no cache hit, first reads up 8 nibbles from the specific address (force the cache hit), then...
// 2) when cache hit occures, then writes back the 32 bits DWORD, but masks the necessary byte of the
// written nible. The mask is 4bit wide, each bits realize one byte in the block memory from the 32bits wide DATA bus.
// CPU have to wait for the read & writeback clocks, therefore CPU could stop input signal will be driven.
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// ADDRESS of IAX bus : 33222222222211111111110000000000|
//                      10987654321098765432109876543210|
//Not used:             00000000000000000000000         |     : 23bits constant zero
//Cache_hit_addrress:                          ccccccccc|...  : 9bits wide: 0..512 : address/8
//Nibble index:                                         |NNN  : 3bits wide: 7..0: 8nibbles stored in 32bits data
//Cached bytes index:                                   |BB.  : 2bits wide: 3..0: 4bytes stored in 32bits data, this is used to calculate a mask for bytes.
//CPU address:                                 AAAAAAAAA|AAA  : msb11..3 is address of block ram, lsb 2..0 is the caches nibble index.
//////////////////////////////////////////////////////////////////////////////////
`define SYS_ADDRESS_WIDTH 32    //outside world's address bus width
`define SYS_ADDRESS_MSB (`SYS_ADDRESS_WIDTH-1)

`define SYS_DATA_WIDTH 32       //outside world's data bus width
`define SYS_DATA_MSB (`SYS_DATA_WIDTH-1)

`define SYS_ADDRESSB_WIDTH 9    //outside world's address bus width, so cpu could access this wide memory block, when addressing 12bit wide internal addr.
`define SYS_ADDRESSB_MSB (`SYS_ADDRESSB_WIDTH-1)

`define SYS_CACHE_SIZE_IN_NIBBLES 8 // 8 nibbles means one cache row.
`define SYS_CACHE_SIZE_IN_BYTES 4   // 4 bytes will compose one cache row.
`define SYS_CACHE_ADDR_WIDTH 3      // log2(8)   : index of 0..7
`define SYS_CACHE_ADDR_MSB 2        // log2(8)-1 : bit0,1,2
`define SYS_BYTEPOSMASK_MSB 3       //4 bytepos for writeback masks (32bit wide bus, 4*8bit writeable data pos)

`define SYS_CACHE_HIT_WIDTH (`CPU_ADDRESS_WIDTH-`SYS_CACHE_ADDR_WIDTH)     // cache address width, to compre the address hit. 12-3=9bits
`define SYS_NOTUSED_ADDRESS_WIDTH 23 //(`SYS_ADDRESS_WIDTH - `SYS_CACHE_ADDRESS_WIDTH) // to fill up the unused bits with zeros. 32-9= 23 bits wide

module sys(
    input reset,    // positive edge, positive level
    input clk,
    output [`SYS_BYTEPOSMASK_MSB:0] bram_web,    //write enables for each bytes (4 signals for 4 consecutive bytes)
    output bram_en,     //one enebles for the bram chip = read all 4 byes.
    output [`SYS_ADDRESS_MSB:0] o_bram_addr, //outside world address bus (cached) here
    input [`SYS_DATA_MSB:0] i_bram_data,     //outside world data bus input direction (cached) here
    output [`SYS_DATA_MSB:0] o_bram_data,     //outside world data bus output direction (cached) here   
    output [`CPU_ADDRESS_MSB:0] o_perif_addr, //outside world address bus (cached) here
    input [`CPU_DATA_MSB:0] i_perif_data,     //outside world data bus input direction (cached) here
    output [`CPU_DATA_MSB:0] o_perif_data,     //outside world data bus output direction (cached) here
`ifdef DEBUG
    output [`SYS_ADDRESSB_MSB:0] init_counter,
    output init,
    output [`CPU_STATE_MSB:0] state,
    output [`CPU_OPCODE_MSB:0] opcode,
    output [`ALU_STATE_MSB:0] alu_cmd,
    output [`CPU_IMMEDIATE_MSB:0] immediate,
    output [`SYS_DATA_MSB:0] dword,
    output [`CPU_ADDRESS_MSB:0] pc,
    output [`CPU_DATA_MSB:0] debug_cpu_data
`endif    
);
    
reg stop;   // stop the cpu = wait state
reg init;    
wire [`CPU_DATA_MSB:0] cpu_o_data;      //comes from cpu , 4 bits wide
wire [`CPU_ADDRESS_MSB:0] cpu_o_addr;   //comes from cpu, 12 bits wide
wire [`CPU_DATA_MSB:0] cpu_i_data;       //comes from system to cpu , 4 bits wide
wire [`CPU_DATA_MSB:0] debug_cpu_data;
assign debug_cpu_data = cpu_i_data;

wire [`SYS_CACHE_ADDR_MSB:0] cpu_addr_nibble;       //low order 3 bits of address from cpu
wire [`SYS_CACHE_HIT_WIDTH:0] cpu_map_addr;         //high order bits of address, which indexes 32bits in memory, and hit of the cache lookup table
//reg  [`SYS_CACHE_HIT_WIDTH:0] mem_level_one_address; // {indexed by addr>>3 from cpu}, this is the hit key of the L1 cache.

wire bram_en;                           //enable block ram, when read or write
wire [`SYS_BYTEPOSMASK_MSB:0] bram_web; // write enable mask. Here the MSB is: sizeof(sys_data)/sizeof(byte)-1:  32/8 -1
wire cpu_rd;                            //cpu want to read from memory/cache
wire we;                                //cpu want to write it's data bus to memory.
wire [`CPU_ADDRESS_MSB:0] o_perif_addr; //cpu address bus, used for periferial address (measured in nibbles, in cpu_data bit width)
wire [`CPU_DATA_MSB:0] i_perif_data;    //cpu input data from periferials
wire [`CPU_DATA_MSB:0] o_perif_data;    //cpu output data to periferials
wire [`SYS_BYTEPOSMASK_MSB:0] abytepos; //address byte pos.

//Internal states and registers
reg [`SYS_DATA_MSB:0] dword;            //internal data cache memory, stores 32 bits, which is stands from 8 nibbles.
wire [`CPU_DATA_MSB:0] nibble [`SYS_CACHE_SIZE_IN_NIBBLES:0]; //the nibbles can be accessed on this wire buses.

wire perif_en;      //periferial access selected. Actually it is not used.
reg [`SYS_ADDRESSB_MSB:0] init_counter; // initial phase use this for pre-init code memory.

//STATIC assignments
assign bram_en= cpu_rd || we || init;            //bram enable, when cpu reads, writes, or pre-init code memory.
assign o_bram_data= (we||init) ? dword : `SYS_DATA_WIDTH'd0;    //write data32 to the block memory
assign o_bram_addr= init? {23'h0, init_counter} : ( bram_en ? {`SYS_NOTUSED_ADDRESS_WIDTH'd0 ,cpu_map_addr} : `SYS_ADDRESS_WIDTH'd0); //block ram address calculation

`ifdef REVERSENIBBLEORDER  //mem addresssing order
assign cpu_addr_nibble= 4'd7 - cpu_o_addr[`SYS_CACHE_ADDR_MSB:0]; //4 low order address bits# 0..3, index for nibbles7..0
`else //bcd order
assign cpu_addr_nibble= cpu_o_addr[`SYS_CACHE_ADDR_MSB:0]; //4 low order address bits# 0..3, index for nibbles0..7
`endif

assign abytepos= cpu_addr_nibble>>1; //index of 3..0 bytes , on 2 bits, means byte pos.
assign bram_web= init? 15 : (we? ( 1<<abytepos ) : 4'd0); // two nibbles index = byte index
assign o_perif_addr= cpu_o_addr; //it is actually the same
assign o_perif_data= cpu_o_data; //it is actually the same
assign perif_en=0; //do not access to perif yet.
assign cpu_map_addr= cpu_o_addr[`CPU_ADDRESS_MSB:3]; //9 high order address bits 12..3. (4096 divided by 8, addr shifted by 3)
assign nibble[0]= dword[`CPU_DATA_MSB:0];
assign nibble[1]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH : `CPU_DATA_WIDTH];
assign nibble[2]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*2 :`CPU_DATA_WIDTH*2];
assign nibble[3]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*3 :`CPU_DATA_WIDTH*3];
assign nibble[4]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*4 :`CPU_DATA_WIDTH*4];
assign nibble[5]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*5 :`CPU_DATA_WIDTH*5];
assign nibble[6]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*6 :`CPU_DATA_WIDTH*6];
assign nibble[7]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*7 :`CPU_DATA_WIDTH*7];

assign cpu_i_data = perif_en? i_perif_data : nibble[cpu_addr_nibble];
//Reset triggers this block
always@(negedge clk)
    begin
        if (reset)
        begin
           stop=1;
           init<=1;
           init_counter<=16;
        end else
        if (stop)
        begin
            if (init_counter)
            begin
                case (init_counter)
                9'h01: dword<= 32'h0f1fff90; 
                9'h02: dword<= 32'h48903803; 
                9'h03: dword<= 32'h1c00206a; 
                9'h04: dword<= 32'h1c01206b; 
                9'h05: dword<= 32'h1c012c02; 
                9'h06: dword<= 32'h1c012c03; 
                9'h07: dword<= 32'h1c01900a; 
                9'h08: dword<= 32'h001c000c; 
                9'h09: dword<= 32'h1c01900e; 
                9'h0a: dword<= 32'h3fff1ffd; 
                9'h0b: dword<= 32'h0f3fffd0; 
                9'h0c: dword<= 32'h522ffdd0; 
                9'h0d: dword<= 32'h48900a90; 
                9'h0e: dword<= 32'h66012345; 
                9'h0f: dword<= 32'h6789a075; 
                9'h10: dword<= 32'h07500000;
                endcase
                init_counter<=init_counter - 9'h1;
            end
            else
            begin
                init<=0;
                stop<=0;
            end
        end else
        begin
       
            if (cpu_rd)
            begin
                if (perif_en)
                begin
//
                end
                else
                begin
                    dword<=i_bram_data;
  //                  cpu_i_data<= nibble[cpu_addr_nibble]; // i_data[`CPU_DATA_MSB:0];
                    if (we) 
                    begin
                        case (cpu_addr_nibble)
                        0: dword[`CPU_DATA_MSB:0]<=cpu_o_data;
                        1: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH : `CPU_DATA_WIDTH]<=cpu_o_data;
                        2: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*2 : `CPU_DATA_WIDTH*2]<=cpu_o_data;
                        3: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*3 : `CPU_DATA_WIDTH*3]<=cpu_o_data;
                        4: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*4 : `CPU_DATA_WIDTH*4]<=cpu_o_data;
                        5: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*5 : `CPU_DATA_WIDTH*5]<=cpu_o_data;
                        6: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*6 : `CPU_DATA_WIDTH*6]<=cpu_o_data;
                        7: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*7 : `CPU_DATA_WIDTH*7]<=cpu_o_data;
                        endcase
                    end
                end
            end
        end
    end

    m_cpu cpu0 (
    .i_reset(reset),
    .i_clock(clk),
    .i_stop(stop),
    .o_addr(cpu_o_addr),
    .o_data(cpu_o_data),
    .i_data(cpu_i_data),
    .o_re(cpu_rd),
    .o_we(we)
    `ifdef DEBUG
    ,.state(state), .opcode(opcode),.alu_cmd(alu_cmd),.immediate(immediate),.pc(pc)
    `endif
    );
    
endmodule
