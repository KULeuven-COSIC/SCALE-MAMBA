
#Compiling script
#
# Run using
#   dc_shell
#
#   source script.tcl
#
#

#Set search path and libraries
set search_path ../
set target_library "NangateOpenCellLibrary_typical_reduced_INV0_XOR0_AND1_NOR2.db"
set link_library "$target_library"

#Define design lib
define_design_lib WORK -path "./work"

#Define top level and arch
set TOPCELL_ENT Keccak_f
set TOPCELL_ARCH Behavioral

#Analyze target design
analyze -library WORK -autoread {./}

#Elaborate design
elaborate $TOPCELL_ENT -architecture $TOPCELL_ARCH -library DEFAULT

#Link design with the library
link

set_dont_touch_network -no_propagate [get_pins *]
set_dont_touch [get_nets {*}]
#Avoid optimization of registers signals

set ungroup_keep_original_design true

#Compile
#compile_ultra -Gateclock -no_autoungroup
compile -exact_map 

#Verilog netlist
######define_name_rules myrules -allowed "A-Za-z0-9_"
######change_names -rules myrules -hierarchy
set verilogout_show_unconnected_pins true
set verilogout_single_bit false
write -format verilog -hierarchy -output Keccak_f_hierarchy.net

#Compile again to get un grouped netlist
compile -exact_map -ungroup_all 

#Verilog netlist
#####define_name_rules myrules -allowed "A-Za-z0-9_"
#####change_names -rules myrules -hierarchy
set verilogout_single_bit false
write -format verilog -hierarchy -output Keccak_f.net

#Reports
report_timing -transition_time -nets -attributes -nosplit > ./reports/timing.log
report_area -nosplit -hierarchy > ./reports/area.log
report_power -nosplit -hier > ./reports/power.log
report_reference -nosplit -hierarchy > ./reports/ref.log
report_resources -nosplit -hierarchy > ./reports/resources.log

