// Code your design here
//`include "../cpu/common.v"
//`include "../cpu/alu.v"
module test(
  i_reset, i_clock
);
input i_reset;
input i_clock;

reg alu_we;
reg alu_re;
reg [`CPU_DATA_MSB:0] alu_data;
reg alu_operand_b;
reg alu_wcarry;
reg [`ALU_STATE_MSB:0] alu_cmd;

wire [`CPU_DATA_MSB:0] alu_o_data;
wire carry;
wire zero;
wire sign;

// 1 cycle ALU operation, with data and b operand stimulus
task alu_op;
  input  [3:0] cmd;         // ALU command
  input  [3:0] acc_in;      // A-register (i_data)
  input        b_in;        // i_operand_b
  input        re_pulse;    // 1, if read-pulse needed
  begin
    alu_data      = acc_in;      // set the ACC
    alu_operand_b = b_in;
    alu_re = 0;                  // read-pulse off
    alu_we = (cmd != `ALU_Nop && cmd != `ALU_Store); // write enabled only if not NOP or Store cycle.
    alu_cmd       = cmd;         // command
    @(posedge i_clock);
    alu_re        = re_pulse;    // optional read-enable
    alu_cmd       = `ALU_Nop;    // back to the idle state
    alu_we        = 0;           // write-enable off
    @(posedge i_clock);
    alu_re        = 0;           // read-pulse off
    @(posedge i_clock);
  end
endtask

always@(posedge i_clock or posedge i_reset)
begin
	if(i_reset)
		begin
        $display("Test reset");
			  alu_cmd= `ALU_Nop;
        alu_data=0;
        alu_operand_b=0;
        alu_wcarry=0;
        alu_we=0;
        alu_re=0;
		end
end
  
m_alu ALU0 (
  .i_reset(i_reset),
  .i_clock(i_clock),
  .i_stop(1'b0),
  .i_we(alu_we),
  .i_re(alu_re),
  .i_data(alu_data),
  .i_operand_b(alu_operand_b),
  //.i_wcarry(alu_wcarry),
  .i_cmd(alu_cmd),
  .o_carry(carry),
  .o_zero(zero),
  .o_sign(sign),
  .o_data(alu_o_data)
);
initial begin
  // after reset
  @(negedge i_reset);    // wait for reset LOW
  repeat(2) @(posedge i_clock);

  // 1) clear
  alu_op(`ALU_Clear, 4'h0, 1'b0, 1'b0);

  // 2) load 0x6, 0x7, 0x8
  alu_op(`ALU_Load, 4'h6, 1'b0, 1'b1);
  alu_op(`ALU_Load, 4'h7, 1'b0, 1'b1);
  alu_op(`ALU_Load, 4'h8, 1'b0, 1'b1);

  // 3) store (peek data from ALU)
  alu_op(`ALU_Store, alu_data, 1'b0, 1'b1);

  // 4) add sequence B=0
  alu_op(`ALU_Add, 4'd5, 1'b0, 1'b0);
  alu_op(`ALU_Add, 4'd3, 1'b0, 1'b0);
  alu_op(`ALU_Add, 4'd2, 1'b0, 1'b0);

  // 5) NOR, NAND example
  alu_op(`ALU_LogicNOR, 4'hA, 1'b0, 1'b0);
  alu_op(`ALU_LogicNAND, 4'h5, 1'b0, 1'b0);

  #20 $finish(2);
end
endmodule