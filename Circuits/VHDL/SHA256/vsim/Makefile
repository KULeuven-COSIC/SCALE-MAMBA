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
#> File name     : Makefile                                                     
#> Time created  : Thu Dec  5 11:40:40 2019                                     
#> Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
#> Details       :                                                              
#>               :                                                              
#> =============================================================================

beh:
	source ~micasusr/design/scripts/questasim_10.6c.rc; vsim -do behavioral.tcl

behcmd:
	source ~micasusr/design/scripts/questasim_10.6c.rc; vsim -c -do behavioral.tcl

syn:
	source ~micasusr/design/scripts/questasim_10.6c.rc; vsim -do synthesized.tcl

syncmd:
	source ~micasusr/design/scripts/questasim_10.6c.rc; vsim -c -do synthesized.tcl

clean:
	rm -rf work transcript vsim.wlf


