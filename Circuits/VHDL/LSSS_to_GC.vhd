library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity LSSS_to_GC is
  generic( size : integer := 512);
  port(    x, y : in STD_LOGIC_VECTOR(size-1 downto 0); -- x, y < p
           p : in STD_LOGIC_VECTOR(size-1 downto 0);
           result : out STD_LOGIC_VECTOR(63 downto 0));
end LSSS_to_GC;

architecture Behavioral of LSSS_to_GC is
  signal t1, t2, s1, s2, z, x_long, y_long, p_long, pdv2_long : STD_LOGIC_VECTOR(size+1 downto 0);
  signal s3, s4 : STD_LOGIC_VECTOR(63 downto 0);
  signal cmp1, cmp2 : STD_LOGIC;
  
begin

  x_long <= "00" & x;
  y_long <= "00" & y;
  p_long <= "00" & p;
  pdv2_long <= "00" & p_long(size downto 1);
 
-- Do the addition operation z=(x+y) mod p   
  t1 <= x_long + y_long;
  t2 <= t1 - p_long;

  cmp1 <= '1' when t1<p else '0';

  mux1: process(t1, t2)
  begin
    if cmp1 = '1' then
      z <= t1(size downto 0);
    else
      z <= t2(size downto 0);
    end if;
  end process;

-- Now negate z if z is bigger than p/2
  s2 <= p_long-z;
  s3 <= s2(63 downto 0);
  s4 <= not(s3)+1;

  cmp2 <= '1' when z<pdv2_long else '0';

  mux2: process(s1,s2)
  begin
    if cmp2 = '1' then
      result <= z(63 downto 0);
    else
      result <= s4;
    end if;
  end process;

end Behavioral;
