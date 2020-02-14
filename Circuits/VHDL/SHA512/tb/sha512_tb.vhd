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
-- File name     : sha512_tb.vhd                                                  
-- Time created  : Wed Dec  4 09:10:46 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                     
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;

entity sha512_tb is end sha512_tb;

architecture test of sha512_tb is

    component sha512 
    port (
        MessageBlockDI : in  std_logic_vector(1023 downto 0);
        PrevHashDI     : in  std_logic_vector(511 downto 0);
        CurrHashDO     : out std_logic_vector(511 downto 0)
    );
    end component;

    signal MessageBlockDI : std_logic_vector(1023 downto 0);
    signal PrevHashDI : std_logic_vector(511 downto 0);
    signal CurrHashDO : std_logic_vector(511 downto 0);

    constant DeltaT : time := 1 ns;
begin
    
    dut: sha512 port map (
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

        MessageBlockDI <= x"6162638000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000018";
        -- $H^(0)$ the initial hash values, order is $H_0^0$, $H_1^0$, ... $H_7^0$
        PrevHashDI <= x"6a09e667f3bcc908bb67ae8584caa73b3c6ef372fe94f82ba54ff53a5f1d36f1510e527fade682d19b05688c2b3e6c1f1f83d9abfb41bd6b5be0cd19137e2179";
        wait for DeltaT;
            assert CurrHashDO = x"ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f"
                report "B.1 FAILED" severity FAILURE;
        report "TEST B.1 PASS";
        -- Example from "APPENDIX B: SHA-256 EXAMPLES",
        -- B.2 SHA-256 Example (Two-Block Message)
        -- https://csrc.nist.gov/csrc/media/publications/fips/180/2/archive/2002-08-01/documents/fips180-2.pdf
        -- B.2.1
        MessageBlockDI <= x"61626364656667686263646566676869636465666768696a6465666768696a6b65666768696a6b6c666768696a6b6c6d6768696a6b6c6d6e68696a6b6c6d6e6f_696a6b6c6d6e6f70_6a6b6c6d6e6f7071_6b6c6d6e6f707172_6c6d6e6f70717273_6d6e6f7071727374_6e6f707172737475_8000000000000000_0000000000000000";
        -- $H^(0)$ the initial hash values, order is $H_0^0$, $H_1^0$, ... $H_7^0$
        --PrevHashDI <= x"5be0cd191f83d9ab9b05688c510e527fa54ff53a3c6ef372bb67ae856a09e667";
        PrevHashDI <= x"6a09e667f3bcc908bb67ae8584caa73b3c6ef372fe94f82ba54ff53a5f1d36f1510e527fade682d19b05688c2b3e6c1f1f83d9abfb41bd6b5be0cd19137e2179";
        wait for DeltaT;
            assert CurrHashDO = x"4319017a2b706e69_cd4b05938bae5e89_0186bf199f30aa95_6ef8b71d2f810585_d787d6764b20bda2_a260144709736920_00ec057f37d14b8e_06add5b50e671c72"
                report "B.2.1 FAILED" severity FAILURE;
        -- B.2.2
        MessageBlockDI <= x"0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000000_0000000000000380";
        -- Hash after the previous block
        PrevHashDI <= x"4319017a2b706e69_cd4b05938bae5e89_0186bf199f30aa95_6ef8b71d2f810585_d787d6764b20bda2_a260144709736920_00ec057f37d14b8e_06add5b50e671c72";
        wait for DeltaT;
            assert CurrHashDO = x"8e959b75dae313da_8cf4f72814fc143f_8f7779c6eb9f7fa1_7299aeadb6889018_501d289e4900f7e4_331b99dec4b5433a_c7d329eeb6dd2654_5e96e55b874be909"
                report "B.2.2 FAILED" severity FAILURE;
        report "TEST B.2 PASS";

        -- all zeros
        MessageBlockDI <= (others => '0');
        -- $H^(0)$ the initial hash values, order is $H_0^0$, $H_1^0$, ... $H_7^0$
        PrevHashDI <= x"6a09e667f3bcc908bb67ae8584caa73b3c6ef372fe94f82ba54ff53a5f1d36f1510e527fade682d19b05688c2b3e6c1f1f83d9abfb41bd6b5be0cd19137e2179";
        wait for DeltaT;

        -- all zeros
        MessageBlockDI <= (others => '0');
        -- $H^(0)$ the initial hash values, order is $H_0^0$, $H_1^0$, ... $H_7^0$
        PrevHashDI <= (others => '0');
        wait for DeltaT;
        report "SUCCESS!";

    end process ; -- stimulus

end architecture ; -- test
