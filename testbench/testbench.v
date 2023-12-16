`timescale 1ns / 1ps

module testbench;

  localparam STEP  = 10; // 10 ns: 100MHz
  localparam STEPV = 14;
  localparam STEPA = 41; // 24.576 MHz
  localparam TICKS = 1000000;

  localparam TRUE = 1'b1;
  localparam FALSE = 1'b0;

  integer i;
  initial
    begin
      $dumpfile("wave.vcd");
      $dumpvars(10, testbench);
      $monitor("time: %d reset: %d", $time, reset);
    end

  // generate clk
  reg clk;
  reg reset;
  initial
    begin
      clk = 1'b1;
      forever
        begin
          #(STEP / 2) clk = ~clk;
        end
    end
  // generate reset signal
  initial
    begin
      reset = 1'b0;
      repeat (60) @(posedge clk) reset <= 1'b1;
      @(posedge clk) reset <= 1'b0;
    end

  // generate clkv
  reg clkv;
  reg resetv;
  initial
    begin
      clkv = 1'b1;
      forever
        begin
          #(STEPV / 2) clkv = ~clkv;
        end
    end

  // generate clka
  reg clka;
  reg reseta;
  initial
    begin
      clka = 1'b1;
      forever
        begin
          #(STEPA / 2) clka = ~clka;
        end
    end

  // stop simulation after TICKS
  initial
    begin
      repeat (TICKS) @(posedge clk);
      $finish;
    end

  // simulate audio fifo
  reg audio_ready;
  initial
    begin
      audio_ready = 1'b0;
      repeat (100) @(posedge clka) audio_ready <= 1'b0;
      repeat (100) @(posedge clka) audio_ready <= 1'b1;
      forever
        begin
          repeat (300) @(posedge clka) audio_ready <= 1'b0;
          //repeat (300) @(posedge clka) audio_ready <= 1'b1;
          @(posedge clka) audio_ready <= 1'b1;
        end
    end

  wire resetn;
  wire resetvn;
  wire resetan;
  assign resetn = ~reset;
  assign resetvn = ~resetv;
  assign resetan = ~reseta;

  rtl_top rtl_top_0
    (
     .clk (clk),
     .clkv (clkv),
     .clka (clka),
     .video_de (),
     .video_hsyncn (),
     .video_vsyncn (),
     .video_color (),
     .audio_data (),
     .audio_id (),
     .audio_valid (),
     .audio_ready (audio_ready),
     .resetn (resetn)
   );

endmodule
