create_clock -period 13.468 -name clk_video -waveform {0.000 6.734} [get_pins design_1_i/zynq_ultra_ps_e_0/inst/PS8_i/DPVIDEOREFCLK]

set_clock_groups -asynchronous -group [get_clocks clk_video] -group [get_clocks -of_objects [get_pins design_1_i/clk_wiz_0/clk_out1]]

set_clock_groups -asynchronous -group [get_clocks -of_objects [get_pins design_1_i/clk_wiz_0/clk_out1]] -group [get_clocks -of_objects [get_pins design_1_i/clk_wiz_1/clk_out1]]

#set_clock_groups -asynchronous -group [get_clocks -of_objects [get_pins design_1_i/clk_wiz_0/clk_out1]] -group [get_clocks -of_objects [get_pins design_1_i/clk_wiz_2/clk_out1]]

#set_clock_groups -asynchronous -group [get_clocks -of_objects [get_pins design_1_i/clk_wiz_1/clk_out1]] -group [get_clocks -of_objects [get_pins design_1_i/clk_wiz_2/clk_out1]]

#create_clock -period 40.690 -name clk_audio -waveform {0.000 20.345} [get_pins design_1_i/zynq_ultra_ps_e_0/inst/PS8_i/DPAUDIOREFCLK]

#set_clock_groups -asynchronous -group [get_clocks clk_audio] -group [get_clocks -of_objects [get_pins design_1_i/clk_wiz_0/clk_out1]]
