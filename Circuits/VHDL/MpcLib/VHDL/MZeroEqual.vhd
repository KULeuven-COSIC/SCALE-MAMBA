library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity MZeroEqual is
generic (
    gWordSize : natural
);
port(  a : in STD_LOGIC_VECTOR(gWordSize - 1 downto 0);
     res : out STD_LOGIC_VECTOR(0 downto 0));
end entity ;

architecture Behavioral of MZeroEqual is
signal v : std_logic_vector(gWordSize - 1 downto 0) := (others => '0'); 
begin
    res <= "1" when a = v else "0";
end Behavioral;
