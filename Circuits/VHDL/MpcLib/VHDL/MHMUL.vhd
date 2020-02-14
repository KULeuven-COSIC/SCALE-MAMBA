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
-- File name     : MHMUL.vhd                                                     
-- Time created  : Fri Jan 24 14:22:34 2020                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity HA is 
port (
    A, B : in std_logic;
    S, Co : out std_logic
);
end entity;

architecture behavioral of HA is
begin
    S <= A xor B;
    Co <= A and B;
end architecture ; -- behavioral

library ieee;
use ieee.std_logic_1164.all;

entity FA is 
port (
    A, B, Ci : in std_logic;
    S, Co : out std_logic
);
end entity;

architecture behavioral of FA is
begin
    S <= A xor B xor Ci;
    Co <= ((A xor Ci) and (B xor Ci)) xor Ci;
end architecture ; -- behavioral


library ieee;
use ieee.std_logic_1164.all;

entity MHMUL is
generic (
    gWordSize : natural
);
port (
    A : in std_logic_vector(gWordSize - 1 downto 0);
    B : in std_logic_vector(gWordSize - 1 downto 0);
    M : out std_logic_vector(gWordSize - 1 downto 0)
);
end entity ; -- MHMUL

architecture structural of MHMUL is

    component HA is 
    port (
        A, B : in std_logic;
        S, Co : out std_logic
    );
    end component;

    component FA is 
    port (
        A, B, Ci : in std_logic;
        S, Co : out std_logic
    );
    end component;

    type StateT  is array(0 to gWordSize - 1) of std_logic_vector(gWordSize - 1 downto 0);
    type AddersT is array(0 to gWordSize - 2) of std_logic_vector(gWordSize - 1 downto 0);

    signal Dots : StateT;
    signal Sums : AddersT;
    signal Cars : AddersT;

begin

    -- dot products
    gDotsB : for i in 0 to gWordSize - 1 generate
        gDotsA : for j in 0 to gWordSize - 1 generate
            Dots(i)(j) <= B(i) and A(j);
        end generate ; -- gDotsA
    end generate ; -- gDots

    -- generate the first adder
    gFirstAdder : for j in 0 to gWordSize - 1 generate
        LSB_HA : if j = 0 generate
            uHA_0 : HA port map(
                A   => Dots(0 + 1)(j    ),  -- current dot product
                B   => Dots(0    )(j + 1),  -- dot product from the previous row
                S   => Sums(0    )(j    ),  -- current sum
                Co  => Cars(0    )(j    )   -- current carry out
            );
        end generate ; -- LSB_HA
        MSB_HA : if j = gWordSize - 1 generate
            uHA_1 : HA port map(
                A   => Dots(0 + 1)(j    ),  -- current dot product
                B   => Cars(0    )(j - 1),  -- current carry in
                S   => Sums(0    )(j    ),  -- current sum
                Co  => Cars(0    )(j    )   -- current carry out
            );
        end generate ; -- FirstAdderMSBHA
        Others_FA : if (j > 0) and (j < gWordSize - 1) generate
            uFA_0 : FA port map(
                A   => Dots(0 + 1)(j    ),  -- current dot product
                B   => Dots(0    )(j + 1),  -- dot product from the previous row
                Ci  => Cars(0    )(j - 1),  -- current carry in
                S   => Sums(0    )(j    ),  -- current sum
                Co  => Cars(0    )(j    )   -- current carry out
            ); 
        end generate ; -- Others_FA
    end generate ; -- gFirstAdder

    -- generate the rest of adders
    gOtherAddersI : for i in 1 to gWordSize - 2 generate
        gOtherAddersJ : for j in 0 to gWordSize - 1 generate
            LSB_HA : if j = 0 generate
                uHA_0 : HA port map(
                    A   => Dots(i + 1)(j    ),  -- current dot product
                    B   => Sums(i - 1)(j + 1),  -- previous sum
                    S   => Sums(i    )(j    ),  -- current sum
                    Co  => Cars(i    )(j    )   -- current carry out
                );
            end generate ; -- LSB_HA
            MSB_FA : if j = gWordSize - 1 generate
                uFA_0 : FA port map(
                    A   => Dots(i + 1)(j    ),  -- current dot product
                    B   => Cars(i - 1)(j    ),  -- out carry from the previous row
                    Ci  => Cars(i    )(j - 1),  -- current carry in
                    S   => Sums(i    )(j    ),  -- current sum
                    Co  => Cars(i    )(j    )   -- current carry out
                ); 
            end generate ; -- FirstAdderMSBHA
            Others_FA : if (j > 0) and (j < gWordSize - 1) generate
                uFA_1 : FA port map(
                    A   => Dots(i + 1)(j    ),  -- current dot product
                    B   => Sums(i - 1)(j + 1),  -- previous sum
                    Ci  => Cars(i    )(j - 1),  -- current carry in
                    S   => Sums(i    )(j    ),  -- current sum
                    Co  => Cars(i    )(j    )   -- current carry out
                ); 
            end generate ; -- Others_FA
        end generate ; -- gOtherAddersJ
    end generate ; -- gOtherAddersI

    M(0) <= Dots(0)(0);
    gWire : for i in 1 to gWordSize - 1 generate
        M(i) <= Sums(i - 1)(0);
    end generate ; -- gWire

end architecture ; -- structural