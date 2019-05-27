library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity sub64 is
  port(  a, b : in STD_LOGIC_VECTOR(63 downto 0);
         result : out STD_LOGIC_VECTOR(63 downto 0));
end sub64;

architecture Behavioral of sub64 is
begin
  result <= a - b;
end Behavioral;