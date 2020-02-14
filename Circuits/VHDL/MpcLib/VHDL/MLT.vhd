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
-- File name     : MLT.vhd                                                     
-- Time created  : Thu Dec 12 09:13:19 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       : Less than operator optimized for MPC
--               : a - b < 0? 
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity MLT is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    B : in std_logic_vector(gWordSize - 1 downto 0);
    LT : out std_logic
  ) ;
end entity ; -- MLT

architecture behavioral of MLT is
    signal Ap : std_logic_vector(gWordSize - 1 downto 0);
    signal AB : std_logic_vector(gWordSize - 1 downto 0);
    signal C : std_logic_vector(gWordSize - 1 downto 0);
    signal AC : std_logic_vector(gWordSize - 2 downto 0);
    signal BC : std_logic_vector(gWordSize - 2 downto 0);

begin
    
    -- invert the minuend A' = ~A
    Ap <= not(A);

    -- initial half-subtractor
    C(0) <= not(A(0)) and B(0);

    -- A xor B
    gAxorB : for i in 0 to gWordSize - 1 generate
        AB(i) <= A(i) xor B(i);
    end generate ; -- gAxorB

    -- full-subtractor cells
    gFullSubtract : for i in 1 to gWordSize - 1 generate
        C(i) <= ((Ap(i) xor C(i - 1)) and (B(i) xor C(i - 1))) xor C(i - 1);
    end generate ; -- gFullSubtract

    -- LT is equal to the final borrow
    LT <= C(gWordSize - 1);
end architecture ; -- behavioral