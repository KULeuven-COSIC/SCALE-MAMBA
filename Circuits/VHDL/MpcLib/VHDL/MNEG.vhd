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
-- File name     : MNEG.vhd                                                     
-- Time created  : Fri Feb  7 08:12:30 2020                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity MNEG is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    O : out std_logic_vector(gWordSize - 1 downto 0)
);
end entity; -- MNEG

architecture behavioral of MNEG is
    signal Ap : std_logic_vector(gWordSize - 1 downto 0);
    signal AB : std_logic_vector(gWordSize - 1 downto 0);
    signal C : std_logic_vector(gWordSize - 2 downto 0);
    signal AC : std_logic_vector(gWordSize - 2 downto 0);
    signal BC : std_logic_vector(gWordSize - 2 downto 0);
    constant B : std_logic_vector(gWordSize - 1 downto 0) := (gWordSize - 1 downto 1 => '0') & '1';

begin
    
    -- binary negation
    Ap <= not A;

    -- initial half-cell
    O(0) <= AB(0);
    C(0) <= Ap(0) and B(0);

    -- A xor B
    gAxorB : for i in 0 to gWordSize - 1 generate
        AB(i) <= A(i) xor B(i);
    end generate ; -- gAxorB

    -- full-cell cells
    gFullCell : for i in 1 to gWordSize - 2 generate
        O(i) <= AB(i) xor C(i - 1);
        C(i) <= ((Ap(i) xor C(i - 1)) and (B(i) xor C(i - 1))) xor C(i - 1);
    end generate ; -- gFullCell

    -- last stage, sum without carry propagation
    O(gWordSize - 1) <= AB(gWordSize - 1) xor C(gWordSize - 2);

end architecture ; -- behavioral