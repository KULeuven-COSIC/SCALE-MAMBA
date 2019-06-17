These are some test circuits in "Bristol Fashion" which
is a modification of the classic "Bristol Format". See
the file
  /src/GC/Circuit.h
for a description of the format.



-------------------------------------------------------------

Instructions for compiling the .vhd files in the directory
VHDL. These are instructions for us in Leuven, useless for
anyone else. But are here, in case we forget.

    ssh vierre64

Then cd to the VHDL directory and make sure the following file is there

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
or
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

To tidy up the VHDL directory type 
    make clean

-------------------------------------------------------------

For each *.net file then run (e.g.)

        ./convert.x adder64

This produces a *.txt file in the Bristol Fashion in subdirectory
Bristol

The list of all main files (excluding test ones) to convert (so far) are....
        ./convert.x adder64
        ./convert.x sub64
        ./convert.x mull2_64
        ./convert.x mult64
        ./convert.x neg64
        ./convert.x zero_equal
        ./convert.x ModAdd512
        ./convert.x LSSS_to_GC
  
