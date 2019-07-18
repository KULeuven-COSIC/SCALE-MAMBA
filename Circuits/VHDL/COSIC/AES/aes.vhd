library ieee;
use ieee.std_logic_1164.all;
use work.constants.all;

entity aes is
    generic (
        Nk : integer;
        Nr : integer
    );
    port (
        key        : in  std_logic_vector(32*Nk-1 downto 0);
        plaintext  : in  std_logic_vector(127 downto 0);

        ciphertext : out std_logic_vector(127 downto 0)
    );
end entity;

architecture rtl of aes is
    signal keys   : key_array;
    signal cts    : ct_array;

    signal sboxo  : std_logic_vector(127 downto 0);
    signal shifto : std_logic_vector(127 downto 0);

    component aes_key is
        generic (
            Nk       : integer;
            word_ctr : integer
        );
        port (
            key1i : in  std_logic_vector(31 downto 0);
            keyki : in  std_logic_vector(31 downto 0);
            keyo  : out std_logic_vector(31 downto 0)
        );
    end component aes_key;

    component aes_enc is
        port (
            key0i : in  std_logic_vector(31 downto 0);
            key1i : in  std_logic_vector(31 downto 0);
            key2i : in  std_logic_vector(31 downto 0);
            key3i : in  std_logic_vector(31 downto 0);
            datai : in  std_logic_vector(127 downto 0);

            datao : out std_logic_vector(127 downto 0)
        );
    end component aes_enc;

    component aes_sbox is
        port (
            datai : in  std_logic_vector(7 downto 0);
            datao : out std_logic_vector(7 downto 0)
        );
    end component aes_sbox;

    component aes_shift is
        port (
            datai : in  std_logic_vector(127 downto 0);
            datao : out std_logic_vector(127 downto 0)
        );
    end component aes_shift;
begin
    key_generate : for i in Nk to (4*(Nr + 1) - 1) generate
        key0 : aes_key
            generic map (
                Nk       => Nk,
                word_ctr => i
            )
            port map (
                key1i => keys(i-1),
                keyki => keys(i-Nk),
                keyo  => keys(i)
            );
    end generate;

    enc_generate : for i in 1 to (Nr - 1) generate
        enc0 : aes_enc
            port map (
                key0i => keys(4*i),
                key1i => keys(4*i+1),
                key2i => keys(4*i+2),
                key3i => keys(4*i+3),
                datai => cts(i-1),

                datao => cts(i)
            );
    end generate;

    sbox_generate : for i in 0 to 15 generate
        sbox0 : aes_sbox
            port map (
                datai => cts(Nr-1)(8*i+7 downto 8*i),
                datao => sboxo(8*i+7 downto 8*i)
            );
    end generate;

    shift0 : aes_shift
        port map (
            datai => sboxo,
            datao => shifto
        );

    keys_process_generate : for i in 0 to (Nk - 1) generate
        keys_process : process(key) is
            variable left  : integer;
            variable right : integer;
        begin
            left    := 32*(Nk - i) - 1;
            right   := left - 31;

            keys(i) <= key(left downto right);
        end process keys_process;
    end generate;

    enc_process : process(keys, plaintext) is
    begin
        cts(0) <= plaintext xor (keys(0) & keys(1) & keys(2) & keys(3));
    end process enc_process;

    ciphertext <= shifto xor (keys(4*Nr) & keys(4*Nr+1) & keys(4*Nr+2) & keys(4*Nr+3));
end architecture rtl;
