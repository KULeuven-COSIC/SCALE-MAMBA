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

entity rho is
	port (
		 rho_in : in  k_state;
		 rho_out: out k_state);
end rho;

architecture Behavioral of rho is

begin
--rho

	i4001: for i in 0 to N-1 generate
		rho_out(0)(0)(i)<=rho_in(0)(0)(i);
	end generate;	
	i4002: for i in 0 to N-1 generate
		rho_out(0)(1)(i)<=rho_in(0)(1)((i-1)mod N);
	end generate;
	i4003: for i in 0 to N-1 generate
		rho_out(0)(2)(i)<=rho_in(0)(2)((i-62)mod N);
	end generate;
	i4004: for i in 0 to N-1 generate
		rho_out(0)(3)(i)<=rho_in(0)(3)((i-28)mod N);
	end generate;
	i4005: for i in 0 to N-1 generate
		rho_out(0)(4)(i)<=rho_in(0)(4)((i-27)mod N);
	end generate;

	i4011: for i in 0 to N-1 generate
		rho_out(1)(0)(i)<=rho_in(1)(0)((i-36)mod N);
	end generate;	
	i4012: for i in 0 to N-1 generate
		rho_out(1)(1)(i)<=rho_in(1)(1)((i-44)mod N);
	end generate;
	i4013: for i in 0 to N-1 generate
		rho_out(1)(2)(i)<=rho_in(1)(2)((i-6)mod N);
	end generate;
	i4014: for i in 0 to N-1 generate
		rho_out(1)(3)(i)<=rho_in(1)(3)((i-55)mod N);
	end generate;
	i4015: for i in 0 to N-1 generate
		rho_out(1)(4)(i)<=rho_in(1)(4)((i-20)mod N);
	end generate;

	i4021: for i in 0 to N-1 generate
		rho_out(2)(0)(i)<=rho_in(2)(0)((i-3)mod N);
	end generate;	
	i4022: for i in 0 to N-1 generate
		rho_out(2)(1)(i)<=rho_in(2)(1)((i-10)mod N);
	end generate;
	i4023: for i in 0 to N-1 generate
		rho_out(2)(2)(i)<=rho_in(2)(2)((i-43)mod N);
	end generate;
	i4024: for i in 0 to N-1 generate
		rho_out(2)(3)(i)<=rho_in(2)(3)((i-25)mod N);
	end generate;
	i4025: for i in 0 to N-1 generate
		rho_out(2)(4)(i)<=rho_in(2)(4)((i-39)mod N);
	end generate;

	i4031: for i in 0 to N-1 generate
		rho_out(3)(0)(i)<=rho_in(3)(0)((i-41)mod N);
	end generate;	
	i4032: for i in 0 to N-1 generate
		rho_out(3)(1)(i)<=rho_in(3)(1)((i-45)mod N);
	end generate;
	i4033: for i in 0 to N-1 generate
		rho_out(3)(2)(i)<=rho_in(3)(2)((i-15)mod N);
	end generate;
	i4034: for i in 0 to N-1 generate
		rho_out(3)(3)(i)<=rho_in(3)(3)((i-21)mod N);
	end generate;
	i4035: for i in 0 to N-1 generate
		rho_out(3)(4)(i)<=rho_in(3)(4)((i-8)mod N);
	end generate;

	i4041: for i in 0 to N-1 generate
		rho_out(4)(0)(i)<=rho_in(4)(0)((i-18)mod N);
	end generate;	
	i4042: for i in 0 to N-1 generate
		rho_out(4)(1)(i)<=rho_in(4)(1)((i-2)mod N);
	end generate;
	i4043: for i in 0 to N-1 generate
		rho_out(4)(2)(i)<=rho_in(4)(2)((i-61)mod N);
	end generate;
	i4044: for i in 0 to N-1 generate
		rho_out(4)(3)(i)<=rho_in(4)(3)((i-56)mod N);
	end generate;
	i4045: for i in 0 to N-1 generate
		rho_out(4)(4)(i)<=rho_in(4)(4)((i-14)mod N);
	end generate;
	
end Behavioral;
