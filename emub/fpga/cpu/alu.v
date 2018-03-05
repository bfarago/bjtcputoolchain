//////////////////////////////////////////////////////////////////////////////////
// Company: MYND-ideal kft 
// Engineer: Barna Farago
// 
// Create Date: 04.03.2018 15:06:24
// Design Name: Xilinx Sys wrapper
// Module Name: m_alu
// Project Name: emub
// Target Devices: ARTIX/7 35T
// Tool Versions: Vivado v2017.4 (64-bit)
// Description: 
// ALU implementation
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////
`include "common.v"

module m_alu(
//input
	i_reset, i_clock, i_stop,
	i_we, i_re,
	i_data,
	i_operand_b,
	i_cmd,
//output
	o_data,
	o_carry,
	o_zero,
	o_sign
);
input	i_reset;
input	i_clock;
input	i_stop;

input	i_we;	//write enable
input	i_re;	//read enable
//input   i_wcarry; //update carry when write
input   [`ALU_STATE_MSB:0] i_cmd;
input	i_operand_b;
input	[`CPU_DATA_MSB:0] i_data; //written data, input
output	[`CPU_DATA_MSB:0] o_data; //written data, input
output o_carry; //operation overflows at most significant bit
output o_zero;  //alu bits are all zeros
output o_sign;  //0:positive, 1:negative


reg		[`ALU_DATA_MSB:0] alu_data; //alu data reg
wire	alu_carry; //one more bit for carry
reg		[`ALU_STATE_MSB:0] state; //alu data reg

//todo: if carry 1, cpu_data is all 0: o_zero must be zero or one? 
assign o_zero= ~|alu_data; //bitwise nor, true only if all bits are zeros
assign o_sign= alu_data[`CPU_DATA_MSB]; //most significant bit on cpu data
assign alu_carry= alu_data[`ALU_DATA_MSB]; //most significant bit on alu data
assign o_carry= alu_carry;
assign o_data= i_re ? alu_data[`CPU_DATA_MSB:0] : `CPU_DATA_WIDTH'bX;

reg reset_state;

always@(negedge i_clock or posedge reset_state)
begin
    if (reset_state)
     state<= `ALU_Nop;
    else
    if(	`ALU_Nop == state)
     state<= i_cmd;
end


always@(posedge i_clock)
begin
	if(i_reset)
		begin
			$display("Alu reset");
			reset_state <=1;
          	alu_data={`ALU_DATA_WIDTH{1'b0}}; // alu_carry=1'b0; also
		end

	if (i_stop)
		$display("ALU stopped");
	else
	begin


    $display("Alu %t d:%h c:%h i:%h o:%h z:%h s:%h c:%h", $time, alu_data, alu_carry, i_data, o_data, o_zero, o_sign, o_carry);
	case (state)
	`ALU_Nop:
		begin
          $display("Alu nop");
		end
	`ALU_Clear:	
		begin
          $display("Alu clear");
          alu_data={`ALU_DATA_WIDTH{1'b0}}; // alu_carry=1'b0;
		  reset_state <=1;
		end		
	`ALU_Load:	
		begin
          $display("Alu load");
		alu_data={1'b0, i_data}; //		alu_carry=0;
		reset_state <=1;
		end
	`ALU_Store:	
		begin
          $display("Alu store");
		reset_state <=1;
		end
	`ALU_Add:	
		begin
          $display("Alu add");
          alu_data[`ALU_DATA_MSB]<=1'b0;  //clear carry
          alu_data=alu_data + {1'b0,i_data} ; 
          alu_data=alu_data + {4'b0,i_operand_b} ; // +1 or +carry
		 reset_state <=1;
		end
	`ALU_ShiftRight:	
		begin
          $display("Alu shiftRight");
		//alu_data=alu_data>>i_data; //todo: clarify how to
		alu_data=i_data>>1; //todo: clarify how to
		reset_state <=1;
		end
	`ALU_LogicNOR:	
		begin
          $display("Alu Nor");
          alu_data[0]=~(alu_data[0]||i_data[0]);
		alu_data[1]=~(alu_data[1]||i_data[1]);
		alu_data[2]=~(alu_data[2]||i_data[2]);
		alu_data[3]=~(alu_data[3]||i_data[3]);
		reset_state <=1;
		end
	`ALU_LogicNAND:	
		begin
          $display("Alu nand");
		alu_data[0]=~(alu_data[0]&&i_data[0]);
		alu_data[1]=~(alu_data[1]&&i_data[1]);
		alu_data[2]=~(alu_data[2]&&i_data[2]);
		alu_data[3]=~(alu_data[3]&&i_data[3]);
		reset_state <=1;
		end		
	endcase
	end
end

endmodule
