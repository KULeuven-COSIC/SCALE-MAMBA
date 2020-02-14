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
#> Details       :                                                              
#>               :                                                              
#> =============================================================================

vlib work

vcom ../src/sha256pkg.vhd
vcom ../src/MADD32.vhd
vcom ../src/Sigma0.vhd
vcom ../src/Sigma1.vhd
vcom ../src/Sum0.vhd
vcom ../src/Sum1.vhd
vcom ../src/Ch.vhd
vcom ../src/Maj.vhd
vcom ../src/MessageSchedule.vhd
vcom ../src/CompressionFunction.vhd
vcom ../src/UpdateFunction.vhd
vcom ../src/sha256.vhd

vcom ../tb/sha256_tb.vhd

vsim -novopt -lib work -t 1ns sha256_tb

add wave /sha256_tb/dut/*

run 4ns
