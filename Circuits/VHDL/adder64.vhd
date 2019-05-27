library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity adder64 is
  port(  a, b : in STD_LOGIC_VECTOR(63 downto 0);
         sum : out STD_LOGIC_VECTOR(63 downto 0));
end adder64;

architecture Behavioral of adder64 is
begin
  sum <= a + b;
end Behavioral;