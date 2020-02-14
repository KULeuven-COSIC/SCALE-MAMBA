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
-- File name     : MSDIV.vhd                                                     
-- Time created  : Thu Dec 12 09:13:19 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       : a / b = q                                                               
--               : Component MADDSUB is an MPC-friendly Ripple Carry Adder/Subtractor                                                              
--               : Component MNEG is a 2's complement negation
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
    signal Op : std_logic_vector(gWordSize - 1 downto 0);
    constant B : std_logic_vector(gWordSize - 1 downto 0) := (gWordSize - 1 downto 1 => '0') & '1';

begin
    
    -- binary negation
    Ap <= not A;
    
    -- initial half-cell
    Op(0) <= AB(0);
    C(0) <= Ap(0) and B(0);

    -- A xor B
    gAxorB : for i in 0 to gWordSize - 1 generate
        AB(i) <= A(i) xor B(i);
    end generate ; -- gAxorB

    -- full-cell cells
    gFullCell : for i in 1 to gWordSize - 2 generate
        Op(i) <= AB(i) xor C(i - 1);
        C(i) <= ((Ap(i) xor C(i - 1)) and (B(i) xor C(i - 1))) xor C(i - 1);
    end generate ; -- gFullCell

    -- last stage, sum without carry propagation
    Op(gWordSize - 1) <= AB(gWordSize - 1) xor C(gWordSize - 2);
    
    O <= not Op;

end architecture ; -- behavioral
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity MADDSUB is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    B : in std_logic_vector(gWordSize - 1 downto 0);
    SubFlag : in std_logic; 
    O : out std_logic_vector(gWordSize - 1 downto 0)
);
end entity; -- MADDSUB

architecture behavioral of MADDSUB is
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
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity MSDIV is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    B : in std_logic_vector(gWordSize - 1 downto 0);
    Q : out std_logic_vector(gWordSize - 1 downto 0)
  ) ;
end entity ; -- MSDIV

architecture behavioral of MSDIV is

    type StateT is array(0 to gWordSize) of std_logic_vector(2 * gWordSize - 1 downto 0);
    signal Remainder1 : StateT;
    signal Remainder2 : StateT;
    signal SubFlagVector : std_logic_vector(0 to gWordSize);

    signal NegA : std_logic_vector(gWordSize - 1 downto 0);
    signal NegB : std_logic_vector(gWordSize - 1 downto 0);
    signal NegQuotient : std_logic_vector(gWordSize - 1 downto 0);
    signal Dividend : std_logic_vector(gWordSize - 1 downto 0);
    signal Divisor : std_logic_vector(gWordSize - 1 downto 0);
    signal Quotient : std_logic_vector(gWordSize downto 0);
    signal DividendSign : std_logic;
    signal DivisorSign : std_logic;
    signal QuotientSign : std_logic;
    constant ZeroWord : std_logic_vector(gWordSize - 1 downto 0) := (others => '0');

begin

    DividendSign <= A(gWordSize - 1);
    DivisorSign <= B(gWordSize - 1);
    QuotientSign <= DividendSign xor DivisorSign;

    -- Make the inputs positive
    uMNEG_Dividend : entity WORK.MNEG generic map (gWordSize => gWordSize) port map (
        A => A, O => NegA
    );
    uM21MUX_Dividend : entity WORK.M21MUX generic map (gWordSize => gWordSize) port map (
        A => NegA, B => A, S => DividendSign, O => Dividend
    );
    uMNEG_Divisor : entity WORK.MNEG generic map (gWordSize => gWordSize) port map (
        A => B, O => NegB
    );
    uM21MUX_Divisor : entity WORK.M21MUX generic map (gWordSize => gWordSize) port map (
        A => NegB, B => B, S => DivisorSign, O => Divisor
    );

    SubFlagVector(0) <= '1'; -- first subtraction
    gSubFlag : for i in 1 to gWordSize generate
        -- if the top bit is 1, it is a negative signed number
        -- then addition needs to be performed (no restore division)
        -- so the SubFlag must be low
        SubFlagVector(i) <= not Remainder1(i - 1)(2 * gWordSize - 1);
    end generate ; -- gSubFlag


    -- First step
    Remainder2(0) <= ZeroWord & Dividend;

    uMADDSUB_0 : entity WORK.MADDSUB generic map (gWordSize => gWordSize) port map (
        A => Remainder2(0)(2 * gWordSize - 1 downto gWordSize),
        B => Divisor,
        SubFlag => SubFlagVector(0), 
        O => Remainder1(0)(2 * gWordSize - 1 downto gWordSize)
    );
    Remainder1(0)(gWordSize - 1 downto 0) <= Remainder2(0)(gWordSize - 1 downto 0);

    -- The rest
    gDivision : for i in 1 to gWordSize generate
        Remainder2(i) <= Remainder1(i - 1)(2 * gWordSize - 1 - 1 downto 0) & '0';
        uMADDSUB : entity WORK.MADDSUB generic map (gWordSize => gWordSize) port map (
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

    uMNEG_Quotient : entity WORK.MNEG generic map (gWordSize => gWordSize) port map (
        A => Quotient(gWordSize - 1 downto 0), O => NegQuotient
    );
    uM21MUX_Quotient : entity WORK.M21MUX generic map (gWordSize => gWordSize) port map (
        A => NegQuotient, B => Quotient(gWordSize - 1 downto 0), S => QuotientSign, O => Q
    );

end architecture ; -- behavioral


