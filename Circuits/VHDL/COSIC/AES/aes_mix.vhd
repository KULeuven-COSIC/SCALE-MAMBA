library ieee;
use ieee.std_logic_1164.all;

entity aes_mix is
    port (
        datai : in  std_logic_vector(127 downto 0);
        datao : out std_logic_vector(127 downto 0)
    );
end entity;

architecture rtl of aes_mix is
    function mul(x : std_logic_vector(7 downto 0); y : std_logic_vector(7 downto 0)) return std_logic_vector is
        variable tmp  : std_logic_vector(7 downto 0);
        variable poly : std_logic_vector(7 downto 0);
    begin
        -- x*byte
        --   = x*(a0 + a1*x + ... + a7*x^7) mod (x^8 + x^4 + x^3 + x + 1)
        --   = (a0*x + a1*x^2 + ... + a6*x^7) + a7*(x^4 + x^3 + x + 1)
        poly := (4 => y(7), 3 => y(7), 1 => y(7), 0 => y(7), others => '0');

        mul_case : case x(3 downto 0) is
            when x"2"   => tmp := (y(6 downto 0) & '0') xor poly;
            when x"3"   => tmp := (y(6 downto 0) & '0') xor poly xor y;

            when others => tmp := (others => '0');
        end case mul_case;

        return tmp;
    end function mul;
begin
    mix_process : process (datai) is
    begin
        datao(127 downto 120) <= mul(x"02", datai(127 downto 120)) xor mul(x"03", datai(119 downto 112)) xor datai(111 downto 104) xor datai(103 downto 96);
        datao(119 downto 112) <= datai(127 downto 120) xor mul(x"02", datai(119 downto 112)) xor mul(x"03", datai(111 downto 104)) xor datai(103 downto 96);
        datao(111 downto 104) <= datai(127 downto 120) xor datai(119 downto 112) xor mul(x"02", datai(111 downto 104)) xor mul(x"03", datai(103 downto 96));
        datao(103 downto 96)  <= mul(x"03", datai(127 downto 120)) xor datai(119 downto 112) xor datai(111 downto 104) xor mul(x"02", datai(103 downto 96));

        datao(95 downto 88)   <= mul(x"02", datai(95 downto 88)) xor mul(x"03", datai(87 downto 80)) xor datai(79 downto 72) xor datai(71 downto 64);
        datao(87 downto 80)   <= datai(95 downto 88) xor mul(x"02", datai(87 downto 80)) xor mul(x"03", datai(79 downto 72)) xor datai(71 downto 64);
        datao(79 downto 72)   <= datai(95 downto 88) xor datai(87 downto 80) xor mul(x"02", datai(79 downto 72)) xor mul(x"03", datai(71 downto 64));
        datao(71 downto 64)   <= mul(x"03", datai(95 downto 88)) xor datai(87 downto 80) xor datai(79 downto 72) xor mul(x"02", datai(71 downto 64));

        datao(63 downto 56)   <= mul(x"02", datai(63 downto 56)) xor mul(x"03", datai(55 downto 48)) xor datai(47 downto 40) xor datai(39 downto 32);
        datao(55 downto 48)   <= datai(63 downto 56) xor mul(x"02", datai(55 downto 48)) xor mul(x"03", datai(47 downto 40)) xor datai(39 downto 32);
        datao(47 downto 40)   <= datai(63 downto 56) xor datai(55 downto 48) xor mul(x"02", datai(47 downto 40)) xor mul(x"03", datai(39 downto 32));
        datao(39 downto 32)   <= mul(x"03", datai(63 downto 56)) xor datai(55 downto 48) xor datai(47 downto 40) xor mul(x"02", datai(39 downto 32));

        datao(31 downto 24)   <= mul(x"02", datai(31 downto 24)) xor mul(x"03", datai(23 downto 16)) xor datai(15 downto 8) xor datai(7 downto 0);
        datao(23 downto 16)   <= datai(31 downto 24) xor mul(x"02", datai(23 downto 16)) xor mul(x"03", datai(15 downto 8)) xor datai(7 downto 0);
        datao(15 downto 8)    <= datai(31 downto 24) xor datai(23 downto 16) xor mul(x"02", datai(15 downto 8)) xor mul(x"03", datai(7 downto 0));
        datao(7 downto 0)     <= mul(x"03", datai(31 downto 24)) xor datai(23 downto 16) xor datai(15 downto 8) xor mul(x"02", datai(7 downto 0));
    end process mix_process;
end architecture rtl;
