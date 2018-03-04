`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: MYND-ideal kft 
// Engineer: Barna Farago
// 
// Create Date: 04.03.2018 15:06:24
// Design Name: Xilinx Sys wrapper
// Module Name: sys
// Project Name: emub
// Target Devices: ARTIX/7 35T
// Tool Versions: Vivado v2017.4 (64-bit)
// Description: 
// connects to dual port bram
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////
`define SYS_ADDRESS_WIDTH 32
`define SYS_ADDRESS_MSB `SYS_ADDRESS_WIDTH-1

`define SYS_DATA_WIDTH 32
`define SYS_DATA_MSB `SYS_DATA_WIDTH-1

module sys(
    input reset,
    input clk,
    output bram_web,
    output bram_en,
    output [`SYS_ADDRESS_MSB:0] o_addr,
    input [`SYS_DATA_MSB:0] i_data,
    output [`SYS_DATA_MSB:0] o_data
    );
    reg stop;
   // wire clk;
    
wire [`CPU_DATA_MSB:0] cpu_o_data;
wire [`CPU_ADDRESS_MSB:0] cpu_o_addr;
reg [`CPU_DATA_MSB:0] cpu_i_data;
wire bram_en;
wire [3:0] bram_web;    // width/8 -1
wire cpu_rd;
assign bram_en= cpu_rd | we;
assign o_data= we ? {28'b0,cpu_o_data} : `SYS_DATA_WIDTH'b0;
assign o_addr= bram_en ? {20'b0 ,cpu_o_addr} : `SYS_ADDRESS_WIDTH'b0;
assign bram_web={3'b0,we};

always@(posedge reset)
    begin
    stop=0;
    end

always@(posedge clk)
    begin
    if (cpu_rd) cpu_i_data= i_data[`CPU_DATA_MSB:0];
    //if (we) data=cpu_o_data;
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
    );
    
endmodule
