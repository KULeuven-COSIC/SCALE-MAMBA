library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
USE IEEE.NUMERIC_STD.ALL;

entity compare is
  port(  a : in STD_LOGIC_VECTOR(63 downto 0);
         b : in STD_LOGIC_VECTOR(63 downto 0);
         res : out STD_LOGIC);
end compare;

architecture Behavioral of compare is
begin
    res <= '1' when signed(a) < signed(b) else '0';
end Behavioral;
