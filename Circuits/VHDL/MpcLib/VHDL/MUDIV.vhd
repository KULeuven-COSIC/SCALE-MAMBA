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
-- File name     : MUDIV.vhd                                                     
-- Time created  : Thu Dec 12 09:13:19 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       : a / b = q                                                               
--               : Component MRCAS is an MPC-friendly Ripple Carry Adder/Subtractor                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity MRCAS is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    B : in std_logic_vector(gWordSize - 1 downto 0);
    SubFlag : in std_logic; 
    O : out std_logic_vector(gWordSize - 1 downto 0)
);
end entity; -- MRCAS

architecture behavioral of MRCAS is
    signal Ap : std_logic_vector(gWordSize - 1 downto 0);
    signal AB : std_logic_vector(gWordSize - 1 downto 0);
    signal C : std_logic_vector(gWordSize - 2 downto 0);
    signal AC : std_logic_vector(gWordSize - 2 downto 0);
    signal BC : std_logic_vector(gWordSize - 2 downto 0);

begin
    
    -- invert the minuend A' = ~A for subtraction (A' = 1 xor A)
    gSelect : for i in 0 to gWordSize - 1 generate
        Ap(i) <= A(i) xor SubFlag;
    end generate ; -- gSelect

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

library ieee;
use ieee.std_logic_1164.all;

entity MUDIV is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    B : in std_logic_vector(gWordSize - 1 downto 0);
    Q : out std_logic_vector(gWordSize - 1 downto 0)
  ) ;
end entity ; -- MUDIV

architecture behavioral of MUDIV is

    component MRCAS is
    generic (
        gWordSize : natural
    );
    port (
        A : in std_logic_vector(gWordSize - 1 downto 0);
        B : in std_logic_vector(gWordSize - 1 downto 0);
        SubFlag : in std_logic; 
        O : out std_logic_vector(gWordSize - 1 downto 0)
    );
    end component;

    type StateT is array(0 to gWordSize) of std_logic_vector(2 * gWordSize - 1 downto 0);
    signal Remainder1 : StateT;
    signal Remainder2 : StateT;
    signal SubFlagVector : std_logic_vector(0 to gWordSize);

    signal Quotient : std_logic_vector(gWordSize downto 0);
    signal Divisor : std_logic_vector(gWordSize - 1 downto 0);
    constant ZeroWord : std_logic_vector(gWordSize - 1 downto 0) := (others => '0');

begin
    
    Divisor <= B;

    SubFlagVector(0) <= '1'; -- first subtraction
    gSubFlag : for i in 1 to gWordSize generate
        -- if the top bit is 1, it is a negative signed number
        -- then addition needs to be performed (no restore division)
        -- so the SubFlag must be low
        SubFlagVector(i) <= not Remainder1(i - 1)(2 * gWordSize - 1);
    end generate ; -- gSubFlag

    -- First step
    Remainder2(0) <= ZeroWord & A;
    uMRCAS_0 : MRCAS generic map (gWordSize => gWordSize) port map (
        A => Remainder2(0)(2 * gWordSize - 1 downto gWordSize),
        B => Divisor,
        SubFlag => SubFlagVector(0), 
        O => Remainder1(0)(2 * gWordSize - 1 downto gWordSize)
    );
    Remainder1(0)(gWordSize - 1 downto 0) <= Remainder2(0)(gWordSize - 1 downto 0);

    -- The rest
    gDivision : for i in 1 to gWordSize generate
        Remainder2(i) <= Remainder1(i - 1)(2 * gWordSize - 1 - 1 downto 0) & '0';
        uMRCAS : MRCAS generic map (gWordSize => gWordSize) port map (
            A => Remainder2(i)(2 * gWordSize - 1 downto gWordSize),
            B => Divisor,
            SubFlag => SubFlagVector(i),
            O => Remainder1(i)(2 * gWordSize - 1 downto gWordSize)
        );
        Remainder1(i)(gWordSize - 1 downto 0) <= Remainder2(i)(gWordSize -1 downto 0);
    end generate ; -- gDivision

    gQuotient : for i in gWordSize downto 0 generate
        Quotient(i) <= not Remainder1(gWordSize - i)(2 * gWordSize - 1);
    end generate ; -- gQuotient

    Q <= Quotient(gWordSize - 1 downto 0);

end architecture ; -- behavioral


