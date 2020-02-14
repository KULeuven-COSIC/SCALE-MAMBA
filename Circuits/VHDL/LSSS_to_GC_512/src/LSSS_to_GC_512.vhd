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
-- File name     : LSSS_to_GC_512.vhd                                                  
-- Time created  : Tue Jan 21 08:38:27 2020                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity LSSS_to_GC_512 is
    generic (
        gWordSize : natural := 512
    );
    port (
        x : in std_logic_vector(gWordSize - 1 downto 0);
        y : in std_logic_vector(gWordSize - 1 downto 0);
        p : in std_logic_vector(gWordSize - 1 downto 0);
        result : out std_logic_vector(63 downto 0)
    );
end LSSS_to_GC_512;

architecture arch of LSSS_to_GC_512 is

    component MADD513 is
    port (
        A : in std_logic_vector(gWordSize downto 0);
        B : in std_logic_vector(gWordSize downto 0);
        S : out std_logic_vector(gWordSize downto 0)
      ) ;
    end component;

    component MSUB513 is
    port (
        A : in std_logic_vector(gWordSize downto 0);
        B : in std_logic_vector(gWordSize downto 0);
        D : out std_logic_vector(gWordSize downto 0)
      ) ;
    end component;

    component MLT513 is
    port (
        A : in std_logic_vector(gWordSize downto 0);
        B : in std_logic_vector(gWordSize downto 0);
        LT : out std_logic
      ) ;
    end component;

    component M21MUX513 is
    port (
        A : in std_logic_vector(gWordSize downto 0);
        B : in std_logic_vector(gWordSize downto 0);
        S : in std_logic;
        O : out std_logic_vector(gWordSize downto 0)
      ) ;
    end component;


    component MADD64 is
    port (
        A : in std_logic_vector(63 downto 0);
        B : in std_logic_vector(63 downto 0);
        S : out std_logic_vector(63 downto 0)
      ) ;
    end component;

    component M21MUX64 is
    port (
        A : in std_logic_vector(63 downto 0);
        B : in std_logic_vector(63 downto 0);
        S : in std_logic;
        O : out std_logic_vector(63 downto 0)
      ) ;
    end component;

    signal xLong :          std_logic_vector(gWordSize downto 0);
    signal yLong :          std_logic_vector(gWordSize downto 0);
    signal pLong :          std_logic_vector(gWordSize downto 0);
    signal t1Long :         std_logic_vector(gWordSize downto 0);
    signal t2Long :         std_logic_vector(gWordSize downto 0);
    signal s1Long :         std_logic_vector(gWordSize downto 0);
    signal s2Long :         std_logic_vector(gWordSize downto 0);
    signal z :              std_logic_vector(gWordSize downto 0);
    signal pdv2Long :       std_logic_vector(gWordSize downto 0);
    signal sumIsLessThanP : std_logic;
    signal isPositive :     std_logic;
    signal s3short :        std_logic_vector(63 downto 0);
    signal s4short :        std_logic_vector(63 downto 0);

begin
    xLong <= "0" & x;
    yLong <= "0" & y;
    pLong <= "0" & p;
    pdv2Long <= "0" & pLong(gWordSize downto 1);

    -- add and reduce if necessary
    uM21MUX_0 : M21MUX513 port map (
        A => t1Long,
        B => t2Long,
        S => sumIsLessThanP,
        O => z
    );
    uMLT_0 : MLT513 port map (
        A => t1Long,
        B => pLong,
        LT => sumIsLessThanP
    );
    uMADD_0 : MADD513 port map (
        A => xLong,
        B => yLong,
        S => t1Long
    );
    uMSUB_0 : MSUB513 port map (
        A => t1Long,
        B => pLong,
        D => t2Long
    );

    -- negate if z is bigger than p/2 (2's complement)
    uM21MUX_1 : M21MUX64 port map (
        A => z(63 downto 0),
        B => s4short,
        S => isPositive,
        O => result
    );
    uMLT_1 : MLT513 port map (
        A => z,
        B => pdv2Long,
        LT => isPositive
    );
    uMSUB_1 : MSUB513 port map (
        A => pLong,
        B => z,
        D => s2Long
    );
    s3short <= not s2Long(63 downto 0);
    uMADD_increment : MADD64 port map (
        A => s3short,
        B => x"0000_0000_0000_0001",
        S => s4short
    );

end architecture ; -- arch
