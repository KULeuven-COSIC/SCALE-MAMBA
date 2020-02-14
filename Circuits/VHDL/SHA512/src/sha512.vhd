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
-- File name     : sha512.vhd                                                   
-- Time created  : Mon Dec  2 10:18:17 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha512pkg.all;

entity sha512 is
port (
    MessageBlockDI : in std_logic_vector(2 * cNWords * cWordWidth - 1 downto 0);
    PrevHashDI : in std_logic_vector(cNWords * cWordWidth - 1 downto 0);
    CurrHashDO : out std_logic_vector(cNWords * cWordWidth - 1 downto 0)
);
end entity;

architecture structural of sha512 is
    
    -- components    
    component MessageSchedule is
    port (
        MessageBlockDI : in tMessageBlock;
        MessageScheduleDO : out tMessageSchedule
    );
    end component;

    component CompressionFunction is
    port (
        MessageScheduleDI : in tMessageSchedule;
        PrevHashDI : in tHashWords;
        CurrStateDO : out tHashWords
    );
    end component;

    component UpdateFunction is
    port (
        PrevHashDI : in tHashWords;
        CurrStateDI : in tHashWords;
        CurrHashDO : out tHashWords 
    );
    end component;

    -- wires
    signal MessageBlockW : tMessageBlock;
    signal MessageScheduleW : tMessageSchedule;
    signal PrevHashW : tHashWords;
    signal CurrStateW : tHashWords;
    signal CurrHashW : tHashWords;

begin
    
    -- wire inputs
    gInputMessage : for i in 2 * cNWords - 1 downto 0 generate
        MessageBlockW(2 * cNWords - 1 - i) <= MessageBlockDI(cWordWidth * (i + 1) - 1 downto cWordWidth * i);
    end generate ; -- gInputMessage

    gInputHash : for i in cNWords - 1 downto 0 generate
        PrevHashW(cNWords - 1 - i) <= PrevHashDI(cWordWidth * (i + 1) - 1 downto cWordWidth * i);
    end generate ; -- gInputHash

    -- wire outputs
    gOutputHash : for i in cNWords - 1 downto 0 generate
        CurrHashDO(cWordWidth * (i + 1) - 1 downto cWordWidth * i) <= CurrHashW(cNWords - 1 - i);
    end generate ; --gOutputHash

    -- connect components
    xMS : MessageSchedule 
    port map (
        MessageBlockDI => MessageBlockW,
        MessageScheduleDO => MessageScheduleW
    );

    xCF : CompressionFunction 
    port map (
        MessageScheduleDI => MessageScheduleW,
        PrevHashDI => PrevHashW,
        CurrStateDO => CurrStateW
    );

    xUF : UpdateFunction 
    port map (
        PrevHashDI => PrevHashW,
        CurrStateDI => CurrStateW,
        CurrHashDO => CurrHashW
    );

end architecture ; -- structural