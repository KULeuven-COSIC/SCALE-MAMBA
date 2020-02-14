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
#> File name     : compile.tcl                                                  
#> Time created  : Thu Dec 12 09:35:20 2019                                     
#> Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
#> Details       :                                                              
#>               :                                                              
#> =============================================================================

set WordSize [get_unix_variable N]
if {$WordSize < 4} {
    return -code error "export N >= 4."
}

set_host_options -max_cores 8
set target_library ../Liberty/MpcLib.db
set link_library $target_library

define_design_lib WORK -path ./WORK

analyze -format vhdl MSDIV.vhd

elaborate MNEG -library WORK -parameters "gWordSize=>$WordSize"
link
uniquify
set_ideal_network [all_inputs]
set_max_area -ignore_tns 0
set_cost_priority -design_rules
report_constraints
compile_ultra
copy_design MNEG_gWordSize$WordSize MNEG

elaborate M21MUX -library WORK -parameters "gWordSize=>$WordSize"
link
uniquify
set_ideal_network [all_inputs]
set_max_area -ignore_tns 0
set_cost_priority -design_rules
report_constraints
compile_ultra
copy_design M21MUX_gWordSize$WordSize M21MUX

elaborate MADDSUB -library WORK -parameters "gWordSize=>$WordSize"
link
uniquify
set_ideal_network [all_inputs]
set_max_area -ignore_tns 0
set_cost_priority -design_rules
report_constraints
compile_ultra
copy_design MADDSUB_gWordSize$WordSize MADDSUB

elaborate MSDIV -library WORK -parameters "gWordSize=>$WordSize"
link
uniquify
set_ideal_network [all_inputs]
set_max_area -ignore_tns 0
set_cost_priority -design_rules
report_constraints
compile_ultra

change_names -hierarchy -rules verilog
ungroup -all -flatten -simple_names
write -format verilog -hierarchy -o MSDIV$WordSize.v

exit
