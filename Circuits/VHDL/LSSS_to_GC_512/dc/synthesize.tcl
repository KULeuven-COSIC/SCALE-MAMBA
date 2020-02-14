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

foreach src [glob ../src/*.v] {
    analyze -format verilog $src
}
analyze -format vhdl ../src/LSSS_to_GC_512.vhd

elaborate LSSS_to_GC_512 -library WORK

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
write -format verilog -hierarchy -o LSSS_to_GC_512.net
exit
