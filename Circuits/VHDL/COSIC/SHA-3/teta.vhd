-- The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
-- Michaël Peeters and Gilles Van Assche. For more information, feedback or
-- questions, please refer to our website: http://keccak.noekeon.org/

-- Implementation by the designers,
-- hereby denoted as "the implementer".

-- To the extent possible under law, the implementer has waived all copyright
-- and related or neighboring rights to the source code in this file.
-- http://creativecommons.org/publicdomain/zero/1.0/

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library work;
	use work.keccak_globals.all;
	
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity teta is
	port (
		 theta_in : in  k_state;
		 theta_out: out k_state);
end teta;

architecture Behavioral of teta is
  signal sum_sheet: k_plane;
begin
	--theta

		--compute sum of columns

		i0101: for x in 0 to 4 generate
			i0102: for i in 0 to N-1 generate
				sum_sheet(x)(i)<=theta_in(0)(x)(i) xor theta_in(1)(x)(i) xor theta_in(2)(x)(i) xor theta_in(3)(x)(i) xor theta_in(4)(x)(i);
			
			end generate;	
		end generate;


		i0200: for y in 0 to 4 generate
			i0201: for x in 1 to 3 generate
				theta_out(y)(x)(0)<=theta_in(y)(x)(0) xor sum_sheet(x-1)(0) xor sum_sheet(x+1)(N-1);
				i0202: for i in 1 to N-1 generate
					theta_out(y)(x)(i)<=theta_in(y)(x)(i) xor sum_sheet(x-1)(i) xor sum_sheet(x+1)(i-1);
				end generate;	
			end generate;
		end generate;

		i2001: for y in 0 to 4 generate
			theta_out(y)(0)(0)<=theta_in(y)(0)(0) xor sum_sheet(4)(0) xor sum_sheet(1)(N-1);
			i2021: for i in 1 to N-1 generate
				theta_out(y)(0)(i)<=theta_in(y)(0)(i) xor sum_sheet(4)(i) xor sum_sheet(1)(i-1);
			end generate;	

		end generate;

		i2002: for y in 0 to 4 generate
			theta_out(y)(4)(0)<=theta_in(y)(4)(0) xor sum_sheet(3)(0) xor sum_sheet(0)(N-1);
			i2022: for i in 1 to N-1 generate
				theta_out(y)(4)(i)<=theta_in(y)(4)(i) xor sum_sheet(3)(i) xor sum_sheet(0)(i-1);
			end generate;	

		end generate;


end Behavioral;

