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
// bram data bus actually 32 bit wide, and cpu is 4 bit wide. The goal of this system interface/bridge to
// realize a virtual 4bit wide memory on the system's 32 bit wide memory. To achive this goal, we are
// implementing a simple read-write cache on 4*bytes = 8 nibbles.
// when cpu read from memory, the sys bridge will read 8 nibbles (if it was not already in the cache).
// in case of the cache hit, no memory read occures.
// When cpu writes a nibble:
// 1) in case of no cache hit, first reads up 8 nibbles, then...
// 2) when cache hit occures, then writes back the 32 bits DWORD, but masks the necessary byte of the
// written nible.
// CPU have to wait for the read & writeback clocks, therefore CPU stop input signal will be driven.
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////
`define SYS_ADDRESS_WIDTH 32    //outside world's address bus width
`define SYS_ADDRESS_MSB `SYS_ADDRESS_WIDTH-1

`define SYS_DATA_WIDTH 32       //outside world's data bus width
`define SYS_DATA_MSB `SYS_DATA_WIDTH-1

`define SYS_CACHE_SIZE_IN_NIBBLES 8 // 8 nibbles means one cache row.
`define SYS_CACHE_SIZE_IN_BYTES 4   // 4 bytes will compose one cache row.
`define SYS_CACHE_MSB 3             // TODO:check this again, I am not sure now :) But log2(cache size which one)-1

`define SYS_BYTEPOSMASK_MSB 3 //4 bytepos for writeback masks (32bit wide bus, 4*8bit writeable data pos)

`define SYS_CACHE_ADDRESS_WIDTH `CPU_ADDRESS_MSB-`SYS_CACHE_MSB     // cache address width, to compre the address hit
`define SYS_NOTUSED_ADDRESS_WIDTH `SYS_ADDRESS_WIDTH - `SYS_CACHE_ADDRESS_WIDTH // to fill up the unused bits with zeros.

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
    output [8:0] init_counter,
    output init,
    output [`CPU_STATE_MSB:0] state
`endif    
);
    
reg stop;   // stop the cpu = wait state
reg init;    
wire [`CPU_DATA_MSB:0] cpu_o_data;      //comes from cpu , 4 bits wide
wire [`CPU_ADDRESS_MSB:0] cpu_o_addr;   //comes from cpu, 12 bits wide
reg [`CPU_DATA_MSB:0] cpu_i_data;       //comes from cpu , 4 bits wide

wire [`SYS_CACHE_MSB:0] cpu_addr_eb;                //low order bits of address from cpu
wire [`SYS_CACHE_ADDRESS_WIDTH:0] cpu_addr;         //high order bits of address

wire bram_en;                           //enable block ram, when read or write
wire [`SYS_BYTEPOSMASK_MSB:0] bram_web; // MSB:sizeof(sys_data)/sizeof(byte)-1:  32/8 -1
wire cpu_rd;                            //cpu want read from memory/cache
wire [`CPU_ADDRESS_MSB:0] o_perif_addr;
wire [`CPU_DATA_MSB:0] i_perif_data;
wire [`CPU_DATA_MSB:0] o_perif_data;

//Internal states and registers
reg [`SYS_DATA_MSB:0] dword;
wire [`CPU_DATA_MSB:0] nibble [`SYS_CACHE_SIZE_IN_NIBBLES:0];
reg [`SYS_CACHE_ADDRESS_WIDTH:0] mem_level_one_address; // {indexed by addr>>3 from cpu}
wire perif_en;

reg [8:0] init_counter;

//STATIC assignments
assign bram_en= cpu_rd | we| init;            //bram enable
assign o_bram_data= we|init ? dword : `SYS_DATA_WIDTH'b0;
//assign o_addr= bram_en ? {20'b0 , o_cpu_addr} : `SYS_ADDRESS_WIDTH'b0;
assign o_bram_addr= init? {32-9'h0,init_counter} : ( bram_en ? {`SYS_NOTUSED_ADDRESS_WIDTH'b0 ,cpu_addr} : `SYS_ADDRESS_WIDTH'b0);
//assign bram_web={3'b0,we};
assign cpu_addr_eb= cpu_o_addr[`SYS_CACHE_MSB:0]; //4 low order address bits# 0..3, index for nibbles0..7
assign cpu_addr= cpu_o_addr[`CPU_ADDRESS_MSB:4]; //8 high order address bits 12..4. (divided by 8)

assign o_perif_addr= cpu_o_addr;
assign o_perif_data= cpu_o_data;
//assign i_perif_data= i_cpu_data;

assign nibble[0]= dword[`CPU_DATA_MSB:0];
assign nibble[1]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH : `CPU_DATA_WIDTH];
assign nibble[2]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*2 :`CPU_DATA_WIDTH*2];
assign nibble[3]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*3 :`CPU_DATA_WIDTH*3];
assign nibble[4]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*4 :`CPU_DATA_WIDTH*4];
assign nibble[5]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*5 :`CPU_DATA_WIDTH*5];
assign nibble[6]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*6 :`CPU_DATA_WIDTH*6];
assign nibble[7]= dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*7 :`CPU_DATA_WIDTH*7];
assign perif_en=0;
assign bram_web= init? 15 : (we? ( cpu_addr_eb>>1 ) : 4'b0);
//Reset triggers this block

always@(posedge clk)
    begin
        if (reset)
        begin
           stop=1;
           init<=1;
           init_counter=16;
        end else
        if (stop)
        begin
            if (init_counter)
            begin
                case (init_counter)
                9'h01: dword= 32'h0f1fff90; 
                9'h02: dword= 32'h48903803; 
                9'h03: dword= 32'h1c00206a; 
                9'h04: dword= 32'h1c01206b; 
                9'h05: dword= 32'h1c012c02; 
                9'h06: dword= 32'h1c012c03; 
                9'h07: dword= 32'h1c01900a; 
                9'h08: dword= 32'h001c000c; 
                9'h09: dword= 32'h1c01900e; 
                9'h0a: dword= 32'h3fff1ffd; 
                9'h0b: dword= 32'h0f3fffd0; 
                9'h0c: dword= 32'h522ffdd0; 
                9'h0d: dword= 32'h48900a90; 
                9'h0e: dword= 32'h66012345; 
                9'h0f: dword= 32'h6789a075; 
                9'h10: dword= 32'h07500000;
                endcase
                init_counter<=init_counter-8'h1;
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
                    cpu_i_data= i_perif_data;
                end
                else
                begin
                    dword<=i_bram_data;
                    cpu_i_data= nibble[cpu_addr_eb]; // i_data[`CPU_DATA_MSB:0];
                    if (we) 
                    begin
                        case (cpu_addr_eb)
                        0: dword[`CPU_DATA_MSB:0]=cpu_o_data;
                        1: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH : `CPU_DATA_WIDTH]=cpu_o_data;
                        2: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*2 : `CPU_DATA_WIDTH*2]=cpu_o_data;
                        3: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*3 : `CPU_DATA_WIDTH*3]=cpu_o_data;
                        4: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*4 : `CPU_DATA_WIDTH*4]=cpu_o_data;
                        5: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*5 : `CPU_DATA_WIDTH*5]=cpu_o_data;
                        6: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*6 : `CPU_DATA_WIDTH*6]=cpu_o_data;
                        7: dword[`CPU_DATA_MSB+`CPU_DATA_WIDTH*7 : `CPU_DATA_WIDTH*7]=cpu_o_data;
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
    ,.state(state)
    `endif
    );
    
endmodule
