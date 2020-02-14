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
-- File name     : M21MUX.vhd                                                     
-- Time created  : Thu Dec 12 09:13:19 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       : Less than operator optimized for MPC
--               : Requires 1 + (n - 1) * 3 AND gates for an n-bit comparison
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity M21MUX is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    B : in std_logic_vector(gWordSize - 1 downto 0);
    S : in std_logic;
    O : out std_logic_vector(gWordSize - 1 downto 0)
  ) ;
end entity ; -- M21MUX

architecture behavioral of M21MUX is
    signal selectA : std_logic_vector(gWordSize - 1 downto 0);
    signal selectB : std_logic_vector(gWordSize - 1 downto 0);
begin
    
    O <= selectA xor selectB;

    gSelect : for i in 0 to gWordSize - 1 generate
        selectA(i) <= A(i) and S;
        selectB(i) <= B(i) and (not S);
    end generate ; -- gSelect


end architecture ; -- behavioral