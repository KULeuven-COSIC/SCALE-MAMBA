-- =============================================================================
--                     This confidential and proprietary code                   
--                       may be used only as authorized by a                    
--                         licensing agreement from                             
--                     KU Leuven, ESAT Department, COSIC Group                  
--                    https://securewww.esat.kuleuven.be/cosic/                 
--                        _____  ____    ____   ____  _____                     
--                       / ___/ / __ \  / __/  /  _/ / ___/                     
--                      / /__  / /_/ / _\ \   _/ /  / /__                       
--                      \___/  \____/ /___/  /___/  \___/                       
--                                                                              
--                              ALL RIGHTS RESERVED                             
--        The entire notice above must be reproduced on all authorized copies.  
-- =============================================================================
-- File name     : Sum1.vhd                                                     
-- Time created  : Fri Dec 13 11:33:13 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha512pkg.all;

entity Sum1 is
port (
    x : in std_logic_vector(cWordWidth - 1 downto 0);
    y : out std_logic_vector(cWordWidth - 1 downto 0)
);
end entity ; -- Sum1

architecture behavioral of Sum1 is

begin

    y <=
        (x(13 downto 0) & x(cWordWidth - 1 downto 14)) xor -- rotr14
        (x(17 downto 0) & x(cWordWidth - 1 downto 18)) xor -- rotr18
        (x(40 downto 0) & x(cWordWidth - 1 downto 41))     -- rotr41
        ; 
     
end architecture ; -- behavioral