# Live Audio Example for AMD (Xilinx) Kria KV260 and KR260

## About this Project

This is an example project that uses the Live Audio and Live Video functions (control of video and audio output from HDMI from the FPGA) on the AMD (Xilinx) Kria KV260 and KR260.

The project is executed in stand-alone (bare metal) mode.

The development environment supports Vitis / Vivado 2023.2 or later.

https://github.com/miya4649/Live_Audio_KV260_KR260_Template/assets/11752987/6a8df6ab-39c3-4977-8055-d6f61f38b995

## Usage

By default, a project is generated for KV260; for KR260, replace "set board_type kv260" in vivado.tcl with "set board_type kr260".

### Synthesis method using make

In a Linux terminal,

$ source "Vitis installation path"/settings64.sh

(e.g. $ source /opt/Xilinx/Vitis/2023.2/settings64.sh )

$ cd "Path of this directory"

$ make

(The entire process is executed at once.)

Next, start the Vitis Unified IDE,

"File": "Open Workspace",

Select the path "(this directory)/vitis_workspace" and OK.

"VIEW": "Flow" to show,

"FLOW": "Component", select "Project_1_app",

"FLOW": "Run" to run it.

### Synthesis method using Vivado's Tcl Console

Start Vivado and in "Menu": "Window": "Tcl Console",

$ pwd

(check the current directory)

$ cd "Path of this directory"

$ source vivado.tcl

(Run the Vivado project generation script)

$ source vivado-run.tcl

(Run Synthesis, Implement, Export HW)

This will generate a Vivado project under the project_1 directory, Bitstream generation and export of XSA files with bitstreams will be performed.

Next, start Vitis Unified IDE and on its Terminal,

$ pwd

(check the current directory)

$ cd Path of this directory

(Go to this directory)

$ vitis -s vitisnew.py

(Run the Vitis project generation script. The build will also be executed automatically)

"File": "Open Workspace"

Select "(this directory)/vitis_workspace" and OK

"View": "Flow" to show,

"FLOW": "Component", select "Project_1_app",

"FLOW": "Run" to run it.

## Supplemental Information

### Updating RTL sub-modules

When only the source code of a submodule of rtl_top.v is modified, the update information may not be captured properly on Vivado. In this case, if you modify rtl_top.v by adding comments or other dummy modifications, Vivado will catch it and display "Refresh Changed Modules", which you can click to reload and reset the source tree.

Example: // rev. 1 (2, 3, ...)
