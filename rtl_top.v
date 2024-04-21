/*
  Copyright (c) 2023, miya
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

module rtl_top
  #(
    parameter SAMPLING_RATE = 44100
    )
  (
   input wire         clk,
   input wire         clkv,
   input wire         clka,
   output wire        video_de,
   output wire        video_hsyncn,
   output wire        video_vsyncn,
   output wire [35:0] video_color,
   output wire [31:0] audio_data,
   output wire        audio_id,
   output wire        audio_valid,
   input wire         audio_ready,
   input wire         resetn
   );

  localparam ZERO = 1'd0;
  localparam ONE = 1'd1;
  localparam TRUE = 1'b1;
  localparam FALSE = 1'b0;
  localparam AUDIO_WIDTH = 16;
  localparam BUFFER_DEPTH = 4;

  // reset
  wire reset;
  wire resetv;
  wire reseta;
  wire resetp;

  assign resetp = ~resetn;

  // synchronize reset signal
  shift_register
    #(
      .DELAY (3)
      )
  shift_register_reset
    (
     .clk (clk),
     .din (resetp),
     .dout (reset)
     );

  shift_register
    #(
      .DELAY (3)
      )
  shift_register_reseta
    (
     .clk (clka),
     .din (resetp),
     .dout (reseta)
     );

  shift_register
    #(
      .DELAY (3)
      )
  shift_register_resetv
    (
     .clk (clkv),
     .din (resetp),
     .dout (resetv)
     );

  // graphics
  wire        video_hsync;
  wire        video_vsync;
  wire [23:0] vga_color_in;
  wire [23:0] vga_color_out;
  reg [7:0]   color_r;
  reg [7:0]   color_g;
  reg [7:0]   color_b;
  wire [10:0] count_h;
  wire [10:0] count_v;
  reg [10:0]  count_line;
  wire [10:0] count_line_v;
  wire        vsync;
  reg         prev_vsync;

  assign video_hsyncn = ~video_hsync;
  assign video_vsyncn = ~video_vsync;
  assign video_color = {vga_color_out[7:0], {4{vga_color_out[0]}}, vga_color_out[23:16], {4{vga_color_out[16]}}, vga_color_out[15:8], {4{vga_color_out[8]}}};

  always @(posedge clk)
    begin
      prev_vsync <= vsync;
    end

  // move white line
  always @(posedge clk)
    begin
      if (reset == TRUE)
        begin
          count_line <= ZERO;
        end
      else
        begin
          if ((vsync == TRUE) && (prev_vsync == FALSE))
            begin
              if (count_line < 1279)
                begin
                  count_line <= count_line + ONE;
                end
              else
                begin
                  count_line <= ZERO;
                end
            end
        end
    end

  cdc_synchronizer
    #(
      .DATA_WIDTH (11)
      )
  cdc_synchronizer_count_line_v
    (
     .data_in (count_line),
     .data_out (count_line_v),
     .clk (clkv)
     );

  // draw color bar
  always @(posedge clkv)
    begin
      if (count_line_v == count_h)
        begin
          // draw white line
          color_r = 255;
          color_g = 255;
          color_b = 255;
        end
      else if (count_h < 256)
        begin
          color_r <= count_h;
          color_g <= ZERO;
          color_b <= ZERO;
        end
      else if ((count_h > 511) && (count_h < 768))
        begin
          color_r <= ZERO;
          color_g <= count_h - 512;
          color_b <= ZERO;
        end
      else if (count_h > 1023)
        begin
          color_r <= ZERO;
          color_g <= ZERO;
          color_b <= count_h - 1024;
        end
      else
        begin
          color_r <= ZERO;
          color_g <= ZERO;
          color_b <= ZERO;
        end
    end

  assign vga_color_in = {color_r, color_g, color_b};

  vga_iface
    #(
      .VGA_MAX_H (1650-1),
      .VGA_MAX_V (750-1),
      .VGA_WIDTH (1280),
      .VGA_HEIGHT (720),
      .VGA_SYNC_H_START (1390),
      .VGA_SYNC_V_START (725),
      .VGA_SYNC_H_END (1430),
      .VGA_SYNC_V_END (730),
      .CLIP_ENABLE (0),
      .CLIP_H_S (0),
      .CLIP_H_E (1280),
      .CLIP_V_S (0),
      .CLIP_V_E (720),
      .PIXEL_DELAY (2),
      .BPP (24)
      )
  vga_iface_0
    (
     .clk (clk),
     .reset (reset),
     .vsync (vsync),
     .vcount (vcount),
     .ext_clkv (clkv),
     .ext_resetv (resetv),
     .ext_color_in (vga_color_in),
     .ext_vga_hs (video_hsync),
     .ext_vga_vs (video_vsync),
     .ext_vga_de (video_de),
     .ext_vga_color_out (vga_color_out),
     .ext_count_h (count_h),
     .ext_count_v (count_v)
     );

  // audio test signal
  reg signed [AUDIO_WIDTH-1:0] sample_l;
  reg signed [AUDIO_WIDTH-1:0] sample_r;
  wire [AUDIO_WIDTH*2-1:0]     sample_data;
  wire                         sample_full;
  reg                          sample_en;
  localparam DECL = (1 << AUDIO_WIDTH) * 440 / SAMPLING_RATE;
  localparam DECR = (1 << AUDIO_WIDTH) * 660 / SAMPLING_RATE;

  assign sample_data = {sample_l, sample_r};

  always @(posedge clk)
    begin
      if (reseta == TRUE)
        begin
          sample_en <= FALSE;
          sample_l <= ZERO;
          sample_r <= ZERO;
        end
      else
        begin
          if (sample_full == FALSE)
            begin
              sample_en <= TRUE;
              sample_l <= sample_l - DECL;
              sample_r <= sample_r - DECR;
            end
          else
            begin
              sample_en <= FALSE;
            end
        end
    end

  xlive_audio
    #(
      .AUDIO_WIDTH (AUDIO_WIDTH),
      .BUFFER_DEPTH (BUFFER_DEPTH)
      )
  xlive_audio_0
    (
     .clk_tx (clka),
     .clk_rx (clk),
     .reset_tx (reseta),
     .reset_rx (reset),
     .data_rx (sample_data),
     .en_rx (sample_en),
     .full_rx (sample_full),
     .data_tx (audio_data),
     .valid_tx (audio_valid),
     .id_tx (audio_id),
     .ready_tx (audio_ready)
     );

endmodule
