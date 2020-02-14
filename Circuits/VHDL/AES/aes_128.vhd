library ieee;
use ieee.std_logic_1164.all;
use work.constants.all;

entity aes_128 is
    port (
        key        : in  std_logic_vector(127 downto 0);
        plaintext  : in  std_logic_vector(127 downto 0);

        ciphertext : out std_logic_vector(127 downto 0)
    );
end entity;

architecture rtl of aes_128 is
    component aes is
        generic (
            Nk : integer := 4;
            Nr : integer := 10
        );
        port (
            key        : in  std_logic_vector(127 downto 0);
            plaintext  : in  std_logic_vector(127 downto 0);

            ciphertext : out std_logic_vector(127 downto 0)
        );
    end component;
begin
    aes0: aes
        port map (
            key        => key,
            plaintext  => plaintext,

            ciphertext => ciphertext
        );
end architecture rtl;
