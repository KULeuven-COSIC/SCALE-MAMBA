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
-- File name     : MessageSchedule.vhd                                          
-- Time created  : Wed Dec  4 15:30:10 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha512pkg.all;

entity MessageSchedule is
port (
    MessageBlockDI : in tMessageBlock;
    MessageScheduleDO : out tMessageSchedule
);
end entity;

architecture structural of MessageSchedule is

    component MADD64
    port (
        A : in std_logic_vector(cWordWidth - 1 downto 0);
        B : in std_logic_vector(cWordWidth - 1 downto 0);
        S : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

    component Sigma0
    port (
        x : in std_logic_vector(cWordWidth - 1 downto 0);
        y : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

    component Sigma1
    port (
        x : in std_logic_vector(cWordWidth - 1 downto 0);
        y : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

    signal MessageScheduleW : tMessageSchedule;
    signal s0 : tWSchedule;
    signal s1 : tWSchedule;
    signal s2 : tWSchedule;
    signal Sigma1W : tWSchedule;
    signal Sigma0W : tWSchedule;

begin
    
    -- wire outputs    
    MessageScheduleDO <= MessageScheduleW;


    gFirst16 : for i in 0 to 2 * cNWords - 1 generate
        MessageScheduleW(i) <= MessageBlockDI(i);
    end generate ; -- gFirst16

    gSigmas : for i in 2 * cNWords to cNRounds - 1 generate
        uSigma0 : Sigma0 port map ( 
            x=>MessageScheduleW(i - 15), 
            y=>Sigma0W(i - 16) 
        );
        uSigma1 : Sigma1 port map ( 
            x=>MessageScheduleW(i -  2), 
            y=>Sigma1W(i - 16) 
        );
    end generate ; -- gSigmas

    gTheRest : for i in 2 * cNWords to cNRounds - 1 generate
        uMADD0 : MADD64 port map ( 
            A=>Sigma1W(i - 16), 
            B=>MessageScheduleW(i - 7), 
            S=>s0(i-16)
        );
        uMADD1 : MADD64 port map ( 
            A=>Sigma0W(i - 16), 
            B=>MessageScheduleW(i - 16), 
            S=>s1(i-16)
        );
        uMADD2 : MADD64 port map ( 
            A=>s0(i - 16), 
            B=>s1(i - 16), 
            S=>MessageScheduleW(i)
        );
    end generate ; -- gTheRest

end architecture ; -- structural

