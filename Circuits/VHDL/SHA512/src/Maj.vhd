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
-- File name     : Maj.vhd                                                      
-- Time created  : Fri Dec 13 11:30:52 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha512pkg.all;

entity Maj is
port (
    a : in std_logic_vector(cWordWidth - 1 downto 0);
    b : in std_logic_vector(cWordWidth - 1 downto 0);
    c : in std_logic_vector(cWordWidth - 1 downto 0);
    o : out std_logic_vector(cWordWidth - 1 downto 0)
);
end entity ; -- Maj

architecture behavioral of Maj is

begin

    o <= ((a xor c) and (b xor c)) xor c;

end architecture ; -- behavioral