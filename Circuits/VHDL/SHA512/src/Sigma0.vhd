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
-- File name     : Sigma0.vhd                                                   
-- Time created  : Thu Dec 12 16:02:05 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha512pkg.all;

entity Sigma0 is
    port (
        x : in std_logic_vector(cWordWidth - 1 downto 0);
        y : out std_logic_vector(cWordWidth - 1 downto 0)
    );
end entity ; -- Sigma0

architecture behavioral of Sigma0 is

begin

    y <= 
        (x(0) & x(cWordWidth - 1 downto 1)) xor          -- rotr1
        (x(7 downto 0) & x(cWordWidth - 1 downto 8)) xor -- rotr8
        ("0000000" & x(cWordWidth - 1 downto 7))         -- shr7
        ;

end architecture ; -- behavioral