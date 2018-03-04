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

always@(posedge i_clock or posedge i_reset)
begin
	if(i_reset)
		begin
        	$display("Test reset");
			alu_cmd= `ALU_Nop;
            alu_data=0;
            alu_operand_b=0;
            alu_wcarry=0;
            alu_we=1;
		end
  #10 alu_cmd=`ALU_Clear;
  #15 alu_re=1;
  #20 alu_cmd=`ALU_Load;
  #30 alu_cmd=`ALU_Store;
  #40 alu_cmd=`ALU_Add;
  #70 $finish(2) ;
end

always@(negedge i_clock) alu_data=alu_data+4'd1;
  
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

endmodule