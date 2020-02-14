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
-- File name     : Sigma1.vhd                                                   
-- Time created  : Thu Dec 12 16:02:08 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha512pkg.all;

entity Sigma1 is
    port (
        x : in std_logic_vector(cWordWidth - 1 downto 0);
        y : out std_logic_vector(cWordWidth - 1 downto 0)
    );
end entity ; -- Sigma1

architecture behavioral of Sigma1 is

begin

    y <=
        (x(18 downto 0) & x(cWordWidth - 1 downto 19)) xor -- rotr19
        (x(60 downto 0) & x(cWordWidth - 1 downto 61)) xor -- rotr61
        ("000000" & x(cWordWidth - 1 downto 6))             -- shr6
        ;

end architecture ; -- behavioral