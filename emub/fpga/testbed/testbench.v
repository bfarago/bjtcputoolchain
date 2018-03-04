// Code your testbench here
// or browse Examples
module tbench_top;
    //clock and reset signal declaration
  reg clk;
  reg reset;
    //clock generation
  always 
  #5 clk = ~clk;
    //reset Generation
  initial begin
    reset = 1;
    clk=0;
    #5 reset =0;
  end
  
  test test0(
    .i_reset(reset),
    .i_clock(clk)
  );
  
    //enabling the wave dump
  initial begin 
    $dumpfile("dump.vcd"); $dumpvars;
    
  end
  
endmodule