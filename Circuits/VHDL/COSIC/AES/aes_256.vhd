library ieee;
use ieee.std_logic_1164.all;
use work.constants.all;

entity aes_256 is
    port (
        key        : in  std_logic_vector(255 downto 0);
        plaintext  : in  std_logic_vector(127 downto 0);

        ciphertext : out std_logic_vector(127 downto 0)
    );
end entity;

architecture rtl of aes_256 is
    component aes is
        generic (
            Nk : integer := 8;
            Nr : integer := 14
        );
        port (
            key        : in  std_logic_vector(255 downto 0);
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
