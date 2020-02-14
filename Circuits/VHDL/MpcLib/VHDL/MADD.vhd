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
-- File name     : MADD.vhd                                                     
-- Time created  : Thu Dec 12 09:13:19 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       : Ripple Carry Adder without output carry for MPC (MPC Add)                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity MADD is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    B : in std_logic_vector(gWordSize - 1 downto 0);
    S : out std_logic_vector(gWordSize - 1 downto 0)
  ) ;
end entity ; -- MADD

architecture behavioral of MADD is

    signal C : std_logic_vector(gWordSize - 2 downto 0);
    signal AC : std_logic_vector(gWordSize - 2 downto 0);
    signal BC : std_logic_vector(gWordSize - 2 downto 0);
    signal AB : std_logic_vector(gWordSize - 1 downto 0);

begin
    
    -- initial half-adder
    S(0) <= AB(0);
    C(0) <= A(0) and B(0);

    -- A xor B
    gAxorB : for i in 0 to gWordSize - 1 generate
        AB(i) <= A(i) xor B(i);
    end generate ; -- gAxorB

    -- full-adder cells
    gFullAdder : for i in 1 to gWordSize - 2 generate
        S(i) <= AB(i) xor C(i - 1);
        C(i) <= ((A(i) xor C(i - 1)) and (B(i) xor C(i - 1))) xor C(i - 1);
    end generate ; -- gFullAdder

    -- last stage, sum without carry propagation
    S(gWordSize - 1) <= AB(gWordSize - 1) xor C(gWordSize - 2);

end architecture ; -- behavioral