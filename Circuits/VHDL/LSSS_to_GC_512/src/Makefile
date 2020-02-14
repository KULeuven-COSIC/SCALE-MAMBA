# =============================================================================
#                     This confidential and proprietary code                   
#                       may be used only as authorized by a                    
#                         licensing agreement from                             
#                     KU Leuven, ESAT Department, COSIC Group                  
#                    https://securewww.esat.kuleuven.be/cosic/                 
#                        _____  ____    ____   ____  _____                     
#                       / ___/ / __ \  / __/  /  _/ / ___/                     
#                      / /__  / /_/ / _\ \   _/ /  / /__                       
#                      \___/  \____/ /___/  /___/  \___/                       
#                                                                              
#                              ALL RIGHTS RESERVED                             
#        The entire notice above must be reproduced on all authorized copies.  
# =============================================================================
# File name     : Makefile                                                     
# Time created  : Fri Jan 31 09:36:02 2020                                     
# Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
# Details       :                                                              
#               :                                                              
# =============================================================================

LibPath := ../../MpcLib/VHDL
Here := $(shell pwd);

get:
	cd $(LibPath); make synth Target=MLT N=513 OutputPath=$(Here)
	cd $(LibPath); make synth Target=MADD N=513 OutputPath=$(Here)
	cd $(LibPath); make synth Target=MSUB N=513 OutputPath=$(Here)
	cd $(LibPath); make synth Target=M21MUX N=513 OutputPath=$(Here)
	cd $(LibPath); make synth Target=M21MUX N=64 OutputPath=$(Here)
	cd $(LibPath); make synth Target=MADD N=64 OutputPath=$(Here)
	cd $(LibPath); make clean

