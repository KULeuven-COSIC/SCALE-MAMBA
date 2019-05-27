library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity mult64 is

  port(  a, b : in STD_LOGIC_VECTOR(63 downto 0);
  result : out STD_LOGIC_VECTOR(63 downto 0));

end mult64;

 
architecture Behavioral of mult64 is
  signal v : STD_LOGIC_VECTOR(127 downto 0);
begin

  v <= a*b;
  result <= v(63 downto 0);

end Behavioral;
