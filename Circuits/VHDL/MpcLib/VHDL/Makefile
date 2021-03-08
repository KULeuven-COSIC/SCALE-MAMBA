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
#> Time created  : Thu Dec 12 09:56:59 2019                                     
#> Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
#> Details       :                                                              
#>               :                                                              
#> =============================================================================

N := 32
Target := MADD
OutputPath := '../Verilog'

synth: 
	export N=$N; \
	source ~micasusr/design/scripts/synopsys_2019.03.rc; \
	dc_shell-t -f $(Target).tcl
	sed -i "s/$(Target)_gWordSize$(N)/$(Target)$(N)/g" $(Target)$(N).v
	mv $(Target)$(N).v $(OutputPath)/$(Target)$(N).v

common:
	$(MAKE) synth Target=MADD N=32
	$(MAKE) synth Target=MADD N=64
	$(MAKE) synth Target=MSUB N=32
	$(MAKE) synth Target=MSUB N=64
	$(MAKE) synth Target=MMUL N=32
	$(MAKE) synth Target=MMUL N=64
	$(MAKE) synth Target=MHMUL N=32
	$(MAKE) synth Target=MHMUL N=64
	$(MAKE) synth Target=MLT N=32
	$(MAKE) synth Target=MLT N=64
	$(MAKE) synth Target=MUDIV N=64
	$(MAKE) synth Target=MSDIV N=64
	$(MAKE) synth Target=MNegate N=64
	$(MAKE) synth Target=MZeroEqual N=64

clean:
	rm -rf *.svf *.log alib-52 WORK
