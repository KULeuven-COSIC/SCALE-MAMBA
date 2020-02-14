#> =============================================================================
#>                     This confidential and proprietary code                   
#>                       may be used only as authorized by a                    
#>                         licensing agreement from                             
#>                     KU Leuven, ESAT Department, COSIC Group                  
#>                    https://securewww.esat.kuleuven.be/cosic/                 
#>                        _____  ____    ____   ____  _____                     
#>                       / ___/ / __ \  / __/  /  _/ / ___/                     
#>                      / /__  / /_/ / _\ \   _/ /  / /__                       
#>                      \___/  \____/ /___/  /___/  \___/                       
#>                                                                              
#>                              ALL RIGHTS RESERVED                             
#>        The entire notice above must be reproduced on all authorized copies.  
#> =============================================================================
#> File name     : synthesize.tcl                                               
#> Time created  : Thu Dec  5 11:59:21 2019                                     
#> Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
#> Details       :                                                              
#>               :                                                              
#> =============================================================================

suppress_message LINT-33
suppress_message LINT-32
suppress_message LINT-29
suppress_message UCN-1

set_host_options -max_cores 4
set target_library ../../MpcLib/Liberty/MpcLib.db
set link_library "$target_library"
define_design_lib WORK -path ./WORK

analyze -format vhdl ../src/sha256pkg.vhd
analyze -format vhdl ../src/MADD32.vhd
analyze -format vhdl ../src/Sigma0.vhd
analyze -format vhdl ../src/Sigma1.vhd
analyze -format vhdl ../src/Sum0.vhd
analyze -format vhdl ../src/Sum1.vhd
analyze -format vhdl ../src/Ch.vhd
analyze -format vhdl ../src/Maj.vhd
analyze -format vhdl ../src/MessageSchedule.vhd
analyze -format vhdl ../src/CompressionFunction.vhd
analyze -format vhdl ../src/UpdateFunction.vhd
analyze -format vhdl ../src/sha256.vhd

# compile the adder first
elaborate MADD32 -library WORK
check_design
link
uniquify
set_ideal_network [all_inputs]
set_max_area -ignore_tns 0
set_cost_priority -design_rules
report_constraints
compile_ultra
change_names -hierarchy -rules verilog
ungroup -all -flatten -simple_names
write -format verilog -hierarchy -o MADD32.v
remove_design

# now the whole circuit
analyze -format verilog MADD32.v
elaborate sha256 -library WORK
check_design
link
uniquify
set_ideal_network [all_inputs]
set_max_area -ignore_tns 0
set_cost_priority -design_rules
report_constraints
compile_ultra

# output
change_names -hierarchy -rules verilog
ungroup -all -flatten -simple_names
write -format verilog -hierarchy -o sha256.net
exit
