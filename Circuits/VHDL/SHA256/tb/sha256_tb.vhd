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
-- File name     : sha256_tb.vhd                                                  
-- Time created  : Wed Dec  4 09:10:46 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                     
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity sha256_tb is end sha256_tb;

architecture test of sha256_tb is

    component sha256 
    port (
        MessageBlockDI : in  std_logic_vector(511 downto 0);
        PrevHashDI     : in  std_logic_vector(255 downto 0);
        CurrHashDO     : out std_logic_vector(255 downto 0)
    );
    end component;

    signal MessageBlockDI : std_logic_vector(511 downto 0);
    signal PrevHashDI : std_logic_vector(255 downto 0);
    signal CurrHashDO : std_logic_vector(255 downto 0);

    constant DeltaT : time := 1 ns;
begin
    
    dut: sha256 port map (
        MessageBlockDI => MessageBlockDI,
        PrevHashDI => PrevHashDI,
        CurrHashDO => CurrHashDO
    );

    stimulus : process
    begin
        -- Example from "APPENDIX B: SHA-256 EXAMPLES",
        -- B.1 SHA-256 Example (One-Block Message)
        -- https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf
        -- B.1
        MessageBlockDI <= x"61626380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000018";
        -- $H^(0)$ the initial hash values, order is $H_0^0$, $H_1^0$, ... $H_7^0$
        --PrevHashDI <= x"5be0cd191f83d9ab9b05688c510e527fa54ff53a3c6ef372bb67ae856a09e667";
        PrevHashDI <= x"6a09e667bb67ae853c6ef372a54ff53a510e527f9b05688c1f83d9ab5be0cd19";
        wait for DeltaT;
            assert CurrHashDO = x"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
                report "B.1 FAILED" severity FAILURE;
        report "TEST B.1 PASS";
        -- Example from "APPENDIX B: SHA-256 EXAMPLES",
        -- B.2 SHA-256 Example (Two-Block Message)
        -- https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf
        -- B.2.1
        MessageBlockDI <= x"6162636462636465636465666465666765666768666768696768696A68696A6B696A6B6C6A6B6C6D6B6C6D6E6C6D6E6F6D6E6F706E6F70718000000000000000";
        -- $H^(0)$ the initial hash values, order is $H_0^0$, $H_1^0$, ... $H_7^0$
        --PrevHashDI <= x"5be0cd191f83d9ab9b05688c510e527fa54ff53a3c6ef372bb67ae856a09e667";
        PrevHashDI <= x"6a09e667bb67ae853c6ef372a54ff53a510e527f9b05688c1f83d9ab5be0cd19";
        wait for DeltaT;
            assert CurrHashDO = x"85e655d6417a17953363376a624cde5c76e09589cac5f811cc4b32c1f20e533a"
                report "B.2.1 FAILED" severity FAILURE;
        -- B.2.2
        MessageBlockDI <= x"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001c0";
        -- Hash after the previous block
        PrevHashDI <= x"85e655d6417a17953363376a624cde5c76e09589cac5f811cc4b32c1f20e533a";
        wait for DeltaT;
            assert CurrHashDO = x"248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"
                report "B.2.2 FAILED" severity FAILURE;
        report "TEST B.2 PASS";
        wait for DeltaT;

        report "SUCCESS!";

    end process ; -- stimulus

end architecture ; -- test