//////////////////////////////////////////////////////////////////////////////////
// Company: MYND-ideal kft 
// Engineer: Barna Farago
// 
// Create Date: 04.03.2018 15:06:24
// Design Name: Xilinx Sys wrapper
// Module Name: m_cpu
// Project Name: emub
// Target Devices: ARTIX/7 35T
// Tool Versions: Vivado v2017.4 (64-bit)
// Description: 
// CPU implementation. Fetch, Decode, Load, Store
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////
`include "common.v"
module m_cpu(
//input
	i_reset, i_clock, i_stop,
	o_addr, o_data, i_data, o_re, o_we
	);
input	i_reset;
input	i_clock;
input	i_stop;
input [`CPU_DATA_MSB:0] i_data;
output reg [`CPU_ADDRESS_MSB:0] o_addr;
output reg [`CPU_DATA_MSB:0] o_data;
output reg o_re;
output reg o_we;

reg [`CPU_STATE_MSB:0] state;
reg [`CPU_ADDRESS_MSB:0] pc;
reg [`CPU_ADDRESS_MSB:0] pc_next;
reg [`CPU_OPCODE_MSB:0] opcode;
reg [`CPU_IMMEDIATE_MSB:0] immediate;
reg [`CPU_ADDRESS_MSB:0] mem_address;
reg [`ALU_STATE_MSB:0] alu_cmd;
reg [`CPU_DATA_MSB:0] w_data_alu;

reg operand_b_alu;
reg we_alu;
reg re_alu;
wire carry;
wire zero;
wire sign;
wire decode_jump;


assign decode_jump = (opcode>=9) ? 1 : 0;
//assign o_addr = mem_address;

always@(negedge i_clock)
begin
	o_re=0;
	o_we=0;
	we_alu=0;
	//we_carry_alu=0;
	operand_b_alu=0;
	alu_cmd=`ALU_Nop;
end

always@(posedge i_clock)
begin
	if(i_reset)
		begin
			$display("reset");
			pc_next=0; //reset vector
			state= `CPUSTATE_JUMP; //jump to
			operand_b_alu=0;
		end
	if (i_stop)
		$display("CPU stopped");
	else
	case (state)
	`CPUSTATE_FETCH0:
		begin
		mem_address=pc;
		o_re=1;
		opcode = i_data;
		state=`CPUSTATE_FETCH1;
		end
	`CPUSTATE_FETCH1:
		begin
		mem_address=pc+1;
		pc_next=pc+2;
		o_re=1;
		//load first nibble, clear the most significant bits.
		immediate<= {`CPU_IMMEDIATE_WIDTH-`CPU_DATA_WIDTH'b0, i_data};
		if (opcode<1) state=`CPUSTATE_DECODE;
		else state=`CPUSTATE_FETCH2;
		end
	`CPUSTATE_FETCH2:
		begin
		mem_address=pc+2;
		o_re=1;
		//keep first nibble, load second nibble, clear the place of the third nibble
		immediate<= {`CPU_DATA_WIDTH'b0, i_data, immediate[`CPU_DATA_MSB:0]};
		state=`CPUSTATE_FETCH3;
		end
	`CPUSTATE_FETCH3:
		begin
		mem_address=pc+3;
		pc_next=pc+4;
		o_re=1;
		// load third nibble
		immediate<= { i_data, immediate[`CPU_DATA_MSB+`CPU_DATA_WIDTH:0]};
		state=`CPUSTATE_DECODE;
		end		
	`CPUSTATE_DECODE:
		begin
			case (opcode)
				`OP_mvia:
					begin
						w_data_alu= immediate;
						//we_carry_alu=1;
						we_alu=1;
						alu_cmd=`ALU_Load;
						state=`CPUSTATE_FETCH0;
					end
				`OP_sta:
					begin
						o_addr=immediate;
						re_alu=1;
						o_data=r_data_alu;
						o_we=1;
						alu_cmd=`ALU_Store;
						state=`CPUSTATE_STORE;
					end
				`OP_lda:
					begin
						o_addr=immediate;
						o_re=1;
						alu_cmd=`ALU_Load;
						state=`CPUSTATE_LOAD;
					end
				`OP_ad0:
					begin
					operand_b_alu=0;
					alu_cmd=`ALU_Add;
					state=`CPUSTATE_JUMP;
					end
				`OP_ad1:
					begin
					operand_b_alu=1;
					alu_cmd=`ALU_Add;
					state=`CPUSTATE_JUMP;
					end
				`OP_adc:
					begin
					operand_b_alu=carry;
					alu_cmd=`ALU_Add;
					state=`CPUSTATE_JUMP;
					end
				`OP_nand:
					begin
					alu_cmd=`ALU_LogicNAND;
					state=`CPUSTATE_JUMP;
					end
				`OP_nor:
					begin
					alu_cmd=`ALU_LogicNOR;
					state=`CPUSTATE_JUMP;
					end
				`OP_rrm:
					begin
					alu_cmd=`ALU_ShiftRight;
					state=`CPUSTATE_JUMP;
					end	
				`OP_jmp:
					begin
					pc_next=immediate;
					state=`CPUSTATE_JUMP;
					end						
				`OP_jz:
					begin
					if (zero) pc_next=immediate;
					state=`CPUSTATE_JUMP;
					end	
				`OP_jnz:
					begin
					if (!zero) pc_next=immediate;
					state=`CPUSTATE_JUMP;
					end						
				`OP_jc:
					begin
					if (carry) pc_next=immediate;
					state=`CPUSTATE_JUMP;
					end	
				`OP_jnc:
					begin
					if (!carry) pc_next=immediate;
					state=`CPUSTATE_JUMP;
					end						
				`OP_jm:
					begin
					if (sign) pc_next=immediate;
					state=`CPUSTATE_JUMP;
					end	
				`OP_jp:
					begin
					if (!sign) pc_next=immediate;
					state=`CPUSTATE_JUMP;
					end	

			endcase
		end
	`CPUSTATE_LOAD:
		begin
		//load data from memory to alu
		w_data_alu <= i_data;
		we_alu=1;
		//we_carry_alu=1;
		pc=pc_next;
		state=`CPUSTATE_FETCH0; //restart cpu cycle
		end
	`CPUSTATE_STORE:
		begin
		//store alu data to memory. (actually started at decode state)
		pc=pc_next;
		state=`CPUSTATE_FETCH0; //restart cpu cycle
		end
	`CPUSTATE_JUMP:
		begin
		//jump to next pc
		pc=pc_next;
		state=`CPUSTATE_FETCH0; //restart cpu cycle
		end
	
	endcase 
end

m_alu ALU0 (
	.i_reset(i_reset), .i_clock(i_clock), .i_stop(i_stop),
	.i_we(we_alu), .i_re(re_alu),
	.i_data(w_data_alu),
	//.i_wcarry(we_carry_alu),
	.i_operand_b(operand_b_alu),
	.o_data(r_data_alu),
	.o_carry(carry),
	.o_zero(zero),
	.o_sign(sign),
	.i_cmd(alu_cmd)
	);


endmodule