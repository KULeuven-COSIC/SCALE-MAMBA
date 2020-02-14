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
-- File name     : UpdateFunction.vhd                                                   
-- Time created  : Fri Dec 13 11:07:03 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha256pkg.all;

entity UpdateFunction is
port (
    PrevHashDI : in tHashWords;
    CurrStateDI : in tHashWords;
    CurrHashDO : out tHashWords 
);
end entity ; -- UpdateFunction

architecture structural of UpdateFunction is

    component MADD32
    port (
        A : in std_logic_vector(cWordWidth - 1 downto 0);
        B : in std_logic_vector(cWordWidth - 1 downto 0);
        S : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

begin

    identifier : for i in 0 to cNWords - 1 generate
        uMADD32 : MADD32 port map (
            A => PrevHashDI(i),
            B => CurrStateDI(i),
            S => CurrHashDO(i)
        );
    end generate ; -- identifier

end architecture ; -- structural