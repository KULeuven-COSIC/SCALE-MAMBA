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
use work.sha256pkg.all;

entity Sum0 is
port (
    x : in std_logic_vector(cWordWidth - 1 downto 0);
    y : out std_logic_vector(cWordWidth - 1 downto 0)
);
end entity ; -- Sum0

architecture behavioral of Sum0 is

begin

    y <= 
        (x(1  downto 0) & x(cWordWidth - 1 downto  2)) xor 
        (x(12 downto 0) & x(cWordWidth - 1 downto 13)) xor 
        (x(21 downto 0) & x(cWordWidth - 1 downto 22))
        ; 

end architecture ; -- behavioral