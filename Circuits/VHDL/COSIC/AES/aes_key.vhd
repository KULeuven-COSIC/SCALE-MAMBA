library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.constants.all;

entity aes_key is
    generic (
        Nk       : integer;
        word_ctr : integer
    );
    port (
        key1i : in  std_logic_vector(31 downto 0);
        keyki : in  std_logic_vector(31 downto 0);
        keyo  : out std_logic_vector(31 downto 0)
    );
end entity;

architecture rtl of aes_key is
    signal rcon   : std_logic_vector(7 downto 0);

    signal sbox0i : std_logic_vector(7 downto 0);
    signal sbox1i : std_logic_vector(7 downto 0);
    signal sbox2i : std_logic_vector(7 downto 0);
    signal sbox3i : std_logic_vector(7 downto 0);

    signal sbox0o : std_logic_vector(7 downto 0);
    signal sbox1o : std_logic_vector(7 downto 0);
    signal sbox2o : std_logic_vector(7 downto 0);
    signal sbox3o : std_logic_vector(7 downto 0);

    component aes_sbox is
        port (
            datai : in  std_logic_vector(7 downto 0);
            datao : out std_logic_vector(7 downto 0)
        );
    end component aes_sbox;
begin
    sbox0 : aes_sbox
        port map (
            datai => sbox0i,
            datao => sbox0o
        );

    sbox1 : aes_sbox
        port map (
            datai => sbox1i,
            datao => sbox1o
        );

    sbox2 : aes_sbox
        port map (
            datai => sbox2i,
            datao => sbox2o
        );

    sbox3 : aes_sbox
        port map (
            datai => sbox3i,
            datao => sbox3o
        );

    rcon_process : process(key1i, keyki) is
    begin
        rcon_case : case word_ctr/Nk is
            when 1      => rcon <= x"01";
            when 2      => rcon <= x"02";
            when 3      => rcon <= x"04";
            when 4      => rcon <= x"08";
            when 5      => rcon <= x"10";
            when 6      => rcon <= x"20";
            when 7      => rcon <= x"40";
            when 8      => rcon <= x"80";
            when 9      => rcon <= x"1b";
            when 10     => rcon <= x"36";

            when others => rcon <= (others => 'X');
        end case rcon_case;
    end process rcon_process;

    expand_process_generate : if not (word_ctr mod Nk = 0) and not (Nk > 6 and word_ctr mod Nk = 4) generate
        expand_process : process(key1i, keyki) is
        begin
            keyo <= keyki xor key1i;
        end process expand_process;
    end generate;

    expand_process_modNk0_generate : if word_ctr mod Nk = 0 and not (Nk > 6 and word_ctr mod Nk = 4) generate
        expand_process : process(key1i, keyki, rcon, sbox0o, sbox1o, sbox2o, sbox3o) is
        begin
            sbox0i <= key1i(23 downto 16);
            sbox1i <= key1i(15 downto 8);
            sbox2i <= key1i(7 downto 0);
            sbox3i <= key1i(31 downto 24);

            keyo   <= keyki xor ((sbox0o xor rcon) & sbox1o & sbox2o & sbox3o);
        end process expand_process;
    end generate;

    expand_process_Nk6_modNk4_generate : if not (word_ctr mod Nk = 0) and (Nk > 6 and word_ctr mod Nk = 4) generate
        expand_process : process(key1i, keyki, sbox0o, sbox1o, sbox2o, sbox3o) is
        begin
            sbox0i <= key1i(31 downto 24);
            sbox1i <= key1i(23 downto 16);
            sbox2i <= key1i(15 downto 8);
            sbox3i <= key1i(7 downto 0);

            keyo   <= keyki xor (sbox0o & sbox1o & sbox2o & sbox3o);
        end process expand_process;
    end generate;
end architecture rtl;
