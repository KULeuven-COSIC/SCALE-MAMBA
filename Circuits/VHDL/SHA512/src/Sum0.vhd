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
-- File name     : Sum0.vhd                                                     
-- Time created  : Fri Dec 13 11:33:10 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha512pkg.all;

entity Sum0 is
port (
    x : in std_logic_vector(cWordWidth - 1 downto 0);
    y : out std_logic_vector(cWordWidth - 1 downto 0)
);
end entity ; -- Sum0

architecture behavioral of Sum0 is

begin

    y <=
        (x(27 downto 0) & x(cWordWidth - 1 downto 28)) xor -- rotr28
        (x(33 downto 0) & x(cWordWidth - 1 downto 34)) xor -- rotr34
        (x(38 downto 0) & x(cWordWidth - 1 downto 39))     -- rotr39
        ; 

end architecture ; -- behavioral