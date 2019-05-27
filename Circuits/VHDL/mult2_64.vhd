library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity mult2_64 is
  port(  a, b : in STD_LOGIC_VECTOR(63 downto 0);
         left, right : out STD_LOGIC_VECTOR(63 downto 0)
         );
end mult2_64;

architecture Behavioral of mult2_64 is
signal x: STD_LOGIC_VECTOR(127 downto 0);
begin
  x <= a * b;
right <= x(63 downto 0);
left <= x(127 downto 64);

end Behavioral;
