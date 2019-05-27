library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity zero_equal is
  port(  a : in STD_LOGIC_VECTOR(63 downto 0);
         res : out STD_LOGIC_VECTOR(0 downto 0));
end zero_equal;

architecture Behavioral of zero_equal is
signal v : std_logic_vector(63 downto 0) := (others => '0'); 
begin
    res <= "1" when a = v else "0";
end Behavioral;
