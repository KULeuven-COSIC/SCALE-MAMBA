These are some test circuits in "Bristol Fashion" which
is a modification of the classic "Bristol Format". See
the file
  /src/GC/Circuit.h
for a description of the format.



-------------------------------------------------------------

For the *old* circuits in directory VHDL, which is basically
only ModAdd512.vhd, which is no longer really used....


These are instructions for us in Leuven, useless for
anyone else. But are here, in case we forget.

    ssh vierre64


cd to the VHDL directory and make sure the following file is there

  NangateOpenCellLibrary_typical_reduced.lib

This produces NOR and INV gates, if you want AND and INV only gates then
use 
  NangateOpenCellLibrary_typical_reduced_INV0_XOR0_AND1_NOR2.lib 
below in every command


Then do

    bash
    source /esat/micas-data/data/design/scripts/lc_2016.12.rc
    lc_shell

    read_lib NangateOpenCellLibrary_typical_reduced.lib
    write_lib NangateOpenCellLibrary -format db -output NangateOpenCellLibrary_typical_reduced.db

    exit

This produces the file

    NangateOpenCellLibrary_typical_reduced.db

or (if used with the AND optimized library)

    NangateOpenCellLibrary_typical_reduced_INV0_XOR0_AND1_NOR2.db


Now do

    source /esat/micas-data/data/design/scripts/synopsys_2016.12.rc

    dc_shell

    set target_library "NangateOpenCellLibrary_typical_reduced.db"
    set link_library "NangateOpenCellLibrary_typical_reduced.db"

Now type (for example), to compile the adder64 program

    analyze -format vhdl adder64.vhd
    elaborate adder64

    compile_ultra -exact_map -area_high_effort_script

    report_area > area.rpt
    write -format verilog -hierarchy -output adder64.net
    exit

This should produce a file called adder64.net

For more complex designs the first two commands (analyze/elaborate)
can be executed more easily via the gui, by logging in using
    ssh -X vierre64
and then using within dc_shell the command
    start_gui

For Keccak do in the Keccak directory
    dc_shell
    source script.tcl
     


To tidy up the VHDL directory type 
    make clean

-------------------------------------------------------------

To compile *all* circuits in VHDL/MpcLib ...

    cd VHDL/MpcLib/VHDL
    make common

The circuits are then placed in VHDL/MpcLib/Verilog

To compile a specific one do...
    make synth Target=<NameOfTheDesign> N=<NumberOfBitsYouWant>

To clean up
    make clean

-------------------------------------------------------------

To compile the AES/SHA-3 circuits do...

   cd VHDL/AES

Change all occurances of aes_XXX in script.tcl to the one you want to compile

Run the script.tcl
   dc_shell
   source script.tcl

To clean up 
   make clean
  
The SHA-3 directory works the same way [but obviously no need
to edit script.tcl]

-------------------------------------------------------------

To compile SHA256/SHA512/LSSS_to_GC_512
   
    cd VHDL/SHA256/dc
    make dc_shell
    make clean

-------------------------------------------------------------

You then need to run ./convert.x on each netlist file. 

As they all live in different places, you need to pull
these from the correct place. The commands for each one
are defined in the script

        ./convert.sh

Which you could just run as a single command.

After running ./convert.x on the netlist file you have
created a *.txt file in the Bristol Fashion in subdirectory
Bristol


-------------------------------------------------------------

There is a program to utilize YoSys synthesis tools to produce
a Bristol Fashion circuit representation. This is untested by the
SCALE maintainers, but has been provided by Mark Will.


./convert_yosys.py --help
usage: convert_yosys.py [-h] [-t TOP_MODULE] [-sy] [-a ADD_FILE]
                        [-l {verilog,vhdl}] [-v]
                        input_file output_file

positional arguments:
  input_file            File to be converted
  output_file           Bristol output file

optional arguments:
  -h, --help            show this help message and exit
  -t TOP_MODULE, --top_module TOP_MODULE
                        Top module of the design
  -sy, --skip_yosys     Input file is now assumed to have been synthesised.
                        This allows you to run Yosys manually
  -a ADD_FILE, --add_file ADD_FILE
                        Add more files to the design
  -l {verilog,vhdl}, --lang {verilog,vhdl}
                        Input langauge. Note vhdl requires Yosys to be built
                        with Verific support, probably better to convert to
                        Verilog for this converter
  -v, --verbose


So a basic use could be the following:

> cat mult64_2.v
module mult64_2 (a,b,res);
    input signed [63:0] a;
    input signed [63:0] b;
    output [127:0] res;
    assign res = (a * b);
endmodule

> ./convert_yosys.py mult64_2.v mult64_2.txt


