library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;


entity divide64 is
  port( a, b : in STD_LOGIC_VECTOR(63 downto 0);
         ans : out STD_LOGIC_VECTOR(63 downto 0));
end divide64;

architecture Behavioral of divide64 is
begin
  ans <= std_logic_vector(signed(a) / signed(b));
end Behavioral;
