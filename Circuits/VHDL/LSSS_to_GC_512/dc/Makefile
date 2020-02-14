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
#> Time created  : Thu Dec  5 11:59:16 2019                                     
#> Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
#> Details       :                                                              
#>               :                                                              
#> =============================================================================

count:
	@echo "#AND2" `cat *.net | grep AND2 | wc -l`
	@echo "#XOR2" `cat *.net | grep XOR2 | wc -l`
	@echo "#INV " `cat *.net | grep INV  | wc -l`
	@echo "#NOR2" `cat *.net | grep NOR2 | wc -l`

dc_shell: 
	source ~micasusr/design/scripts/synopsys_2019.03.rc; time dc_shell -f synthesize.tcl

clean:
	rm -rf alib-52 *WORK default.svf *.log *.syn *.mr *.pvl ARCH PACK BODY ENTI *.v
