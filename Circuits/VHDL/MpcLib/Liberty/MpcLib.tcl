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
# File name     : MpcLib.tcl                                                  
# Time created  : Thu Dec 12 08:55:12 2019                                     
# Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
# Details       :                                                              
#               :                                                              
# =============================================================================

set search_path .
read_lib MpcLib.lib
write_lib MpcLib -format db -o MpcLib.db
exit
