library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity neg64 is
  port(  a: in STD_LOGIC_VECTOR(63 downto 0);  
  result: out STD_LOGIC_VECTOR(63 downto 0));
end neg64;

architecture Behavioral of neg64 is
begin
  result <= not(a)+1;
end Behavioral;
