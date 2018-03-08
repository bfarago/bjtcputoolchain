// Common defines
`define DEBUG 1
`define ALU_DATA_WIDTH 5    // carry+cpu_data
`define ALU_STATE_WIDTH 3
`define CPU_STATE_WIDTH 3
`define CPU_DATA_WIDTH 4
`define CPU_OPCODE_WIDTH 4
`define CPU_ADDRESS_WIDTH 12
`define CPU_IMMEDIATE_WIDTH 12

`define ALU_DATA_MSB (`ALU_DATA_WIDTH-1)
`define ALU_STATE_MSB (`ALU_STATE_WIDTH-1)
`define CPU_STATE_MSB (`CPU_STATE_WIDTH-1)
`define CPU_DATA_MSB (`CPU_DATA_WIDTH-1)
`define CPU_OPCODE_MSB (`CPU_OPCODE_WIDTH-1)
`define CPU_ADDRESS_MSB (`CPU_ADDRESS_WIDTH-1)
`define CPU_IMMEDIATE_MSB (`CPU_IMMEDIATE_WIDTH-1)

// all modules must implement the following control bus:
//  input reset		pos edge reset
//  input clock		pos edge clk
//	input stop		1: syncron stop, for debug. 0:run

//ALU states/commands
`define ALU_Nop			0
`define ALU_Clear		1
`define ALU_Load		2
`define ALU_Store		3
`define ALU_Add			4
`define ALU_ShiftRight	5
`define ALU_LogicNOR	6
`define ALU_LogicNAND	7

//CPU states
`define CPUSTATE_FETCH0	0
`define CPUSTATE_FETCH1	1
`define CPUSTATE_FETCH2	2
`define CPUSTATE_FETCH3	3
`define CPUSTATE_LOAD	4
`define CPUSTATE_DECODE	5
`define CPUSTATE_STORE	6
`define CPUSTATE_JUMP	7

//CPU opcodes
`define OP_mvia	0
`define OP_sta	1
`define OP_lda	2
`define OP_ad0	3
`define OP_ad1	4
`define OP_adc	5
`define OP_nand	6
`define OP_nor	7
`define OP_rrm	8
`define OP_jmp	9
`define OP_jc	10
`define OP_jnc	11
`define OP_jz	12
`define OP_jnz	13
`define OP_jm	14
`define OP_jp	15

