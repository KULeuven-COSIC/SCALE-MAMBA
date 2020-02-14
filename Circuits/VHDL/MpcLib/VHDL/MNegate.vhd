library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity MNegate is
generic (
    gWordSize : natural
);
port(  a: in STD_LOGIC_VECTOR(gWordSize - 1 downto 0);  
  result: out STD_LOGIC_VECTOR(gWordSize - 1 downto 0));
end entity ;

architecture Behavioral of MNegate is
begin
  result <= not(a)+1;
end Behavioral;
