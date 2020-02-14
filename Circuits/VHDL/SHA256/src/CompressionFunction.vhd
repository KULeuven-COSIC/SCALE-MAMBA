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
-- File name     : CompressionFunction.vhd                                      
-- Time created  : Wed Dec  4 16:15:13 2019                                     
-- Author        : Danilo Sijacic (dsijacic@esat.kuleuven.be)                   
-- Details       :                                                              
--               :                                                              
-- =============================================================================

library ieee;
use ieee.std_logic_1164.all;
use work.sha256pkg.all;

entity CompressionFunction is
port (
    MessageScheduleDI : in tMessageSchedule;
    PrevHashDI : in tHashWords;
    CurrStateDO : out tHashWords
);
end entity;

architecture behavioral_generic of CompressionFunction is

    component MADD32
    port (
        A : in std_logic_vector(cWordWidth - 1 downto 0);
        B : in std_logic_vector(cWordWidth - 1 downto 0);
        S : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

    component Sum0 is
    port (
        x : in std_logic_vector(cWordWidth - 1 downto 0);
        y : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

    component Sum1 is
    port (
        x : in std_logic_vector(cWordWidth - 1 downto 0);
        y : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

    component Maj is
    port (
        a : in std_logic_vector(cWordWidth - 1 downto 0);
        b : in std_logic_vector(cWordWidth - 1 downto 0);
        c : in std_logic_vector(cWordWidth - 1 downto 0);
        o : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

    component Ch is
    port (
        e : in std_logic_vector(cWordWidth - 1 downto 0);
        f : in std_logic_vector(cWordWidth - 1 downto 0);
        g : in std_logic_vector(cWordWidth - 1 downto 0);
        o : out std_logic_vector(cWordWidth - 1 downto 0)
    );
    end component;

    signal a : tUnrolledWord;
    signal b : tUnrolledWord;
    signal c : tUnrolledWord;
    signal d : tUnrolledWord;
    signal e : tUnrolledWord;
    signal f : tUnrolledWord;
    signal g : tUnrolledWord;
    signal h : tUnrolledWord;

    signal T1W : tMessageSchedule;
    signal T2W : tMessageSchedule;
    signal Sum0W : tMessageSchedule;
    signal Sum1W : tMessageSchedule;
    signal ChW : tMessageSchedule;
    signal MajW : tMessageSchedule;
    signal T1AddW0 : tMessageSchedule;
    signal T1AddW1 : tMessageSchedule;
    signal T1AddW2 : tMessageSchedule;


begin

    -- wire io
    a(0) <= PrevHashDI(0);
    b(0) <= PrevHashDI(1);
    c(0) <= PrevHashDI(2);
    d(0) <= PrevHashDI(3);
    e(0) <= PrevHashDI(4);
    f(0) <= PrevHashDI(5);
    g(0) <= PrevHashDI(6);
    h(0) <= PrevHashDI(7);
    CurrStateDO(0) <= a(cNRounds);
    CurrStateDO(1) <= b(cNRounds);
    CurrStateDO(2) <= c(cNRounds);
    CurrStateDO(3) <= d(cNRounds);
    CurrStateDO(4) <= e(cNRounds);
    CurrStateDO(5) <= f(cNRounds);
    CurrStateDO(6) <= g(cNRounds);
    CurrStateDO(7) <= h(cNRounds);

    gUnrollT : for t in 0 to cNRounds - 1 generate
        -- t1
        uSum1 : Sum1 port map (
            x => e(t),
            y => Sum1W(t)
        );
        uCh : Ch port map (
            e => e(t),
            f => f(t),
            g => g(t),
            o => ChW(t)
        );
        uMADDt1_0 : MADD32 port map (
            A => Sum1W(t),
            B => ChW(t),
            S => T1AddW0(t)
        );
        uMADDt1_1 : MADD32 port map (
            A => MessageScheduleDI(t),
            B => h(t),
            S => T1AddW1(t)
        );
        uMADDt1_2 : MADD32 port map (
            A => T1AddW0(t),
            B => T1AddW1(t),
            S => T1AddW2(t)
        );
        uConstMADDt_0 : MADD32 port map (
            A => T1AddW2(t),
            B => cKData(t),
            S => T1W(t)
        );

        -- t2
        uSum0 : Sum0 port map (
            x => a(t),
            y => Sum0W(t)
        );
        uMaj : Maj port map (
            a => a(t),
            b => b(t),
            c => c(t),
            o => MajW(t)
        );
        uMADDt2_0 : MADD32 port map (
            A => Sum0W(t),
            B => MajW(t),
            S => T2W(t)
        );

    end generate ; -- gTemps

    gRounds : for t in 1 to cNRounds generate
        uMADDa : MADD32 port map ( 
            A => T1W(t - 1), 
            B => T2W(t - 1), 
            S => a(t) 
        );
        b(t) <= a(t - 1);
        c(t) <= b(t - 1);
        d(t) <= c(t - 1);
        uMADDe : MADD32 port map ( 
            A => d(t - 1), 
            B => T1W(t - 1), 
            S => e(t) 
        );
        f(t) <= e(t - 1);
        g(t) <= f(t - 1);
        h(t) <= g(t - 1);
    end generate ; -- gRounds


end architecture ; -- generic