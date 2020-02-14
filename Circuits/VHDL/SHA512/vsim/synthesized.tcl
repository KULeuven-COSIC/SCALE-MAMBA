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
#> File name     : behavioral.do                                                
#> Time created  : Wed Dec  4 14:52:35 2019                                     
#> Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                      
#> Details       : Zero delay simulation.
#>               :                                                              
#> =============================================================================

vlib work

vlog ../../MpcLib/Verilog/MpcLib.v +time_mode_zero
vlog ../dc/sha512.net
vcom -explicit -93 ../tb/sha512_tb.vhd

vsim -lib work -t 1ns sha512_tb

add wave /sha512_tb/dut/*
vcd file ../tb/sha512_tb.vcd
vcd add /sha512_tb/dut/*
run 6ns
