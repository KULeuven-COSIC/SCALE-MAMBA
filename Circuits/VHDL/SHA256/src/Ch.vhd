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
-- File name     : Ch.vhd                                                       
-- Time created  : Fri Dec 13 11:39:25 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha256pkg.all;

entity Ch is
port (
    e : in std_logic_vector(cWordWidth - 1 downto 0);
    f : in std_logic_vector(cWordWidth - 1 downto 0);
    g : in std_logic_vector(cWordWidth - 1 downto 0);
    o : out std_logic_vector(cWordWidth - 1 downto 0)
);
end entity ; -- Ch

architecture behavioral of Ch is

begin

    o <= ((f xor g) and e) xor g;
    
end architecture ; -- behavioral