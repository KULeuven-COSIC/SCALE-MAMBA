library ieee;
use ieee.std_logic_1164.all;

package constants is
    type key_array is array(60 downto 0) of std_logic_vector(31 downto 0);
    type ct_array is array(13 downto 0) of std_logic_vector(127 downto 0);
end package;
