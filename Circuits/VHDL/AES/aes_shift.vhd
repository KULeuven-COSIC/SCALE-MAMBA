library ieee;
use ieee.std_logic_1164.all;

entity aes_shift is
    port (
        datai : in  std_logic_vector(127 downto 0);
        datao : out std_logic_vector(127 downto 0)
    );
end entity;

architecture rtl of aes_shift is
begin
    shift_process : process (datai) is
    begin
        datao(127 downto 120) <= datai(127 downto 120);
        datao(119 downto 112) <= datai(87 downto 80);
        datao(111 downto 104) <= datai(47 downto 40);
        datao(103 downto 96)  <= datai(7 downto 0);
        
        datao(95 downto 88)   <= datai(95 downto 88);
        datao(87 downto 80)   <= datai(55 downto 48);
        datao(79 downto 72)   <= datai(15 downto 8);
        datao(71 downto 64)   <= datai(103 downto 96);
        
        datao(63 downto 56)   <= datai(63 downto 56);
        datao(55 downto 48)   <= datai(23 downto 16);
        datao(47 downto 40)   <= datai(111 downto 104);
        datao(39 downto 32)   <= datai(71 downto 64);
        
        datao(31 downto 24)   <= datai(31 downto 24);
        datao(23 downto 16)   <= datai(119 downto 112);
        datao(15 downto 8)    <= datai(79 downto 72);
        datao(7 downto 0)     <= datai(39 downto 32);        
    end process shift_process;
end architecture rtl;
