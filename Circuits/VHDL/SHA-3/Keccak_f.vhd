-- =============================================================================
--                     This confidential and proprietary code                   
--                       may be used only as authorized by a                    
--                           licensing agreement from                             
--                     KU Leuven, ESAT Department, COSIC Group                  
--                    https://securewww.esat.kuleuven.be/cosic/                 
--                        _____  ____    ____   ____  _____                     
--                       / ___/ / __ \  / __/  /  _/ / ___/                     
--                      / /__  / /_/ / _\ \   _/ /  / /__                       
--                      \___/  \____/ /___/  /___/  \___/                       
--                                                                              
--                              ALL RIGHTS RESERVED                             
--        The entire notice above must be reproduced on all authorized copies.  
-- =============================================================================
-- File name     : Keccak_f.vhd                                                
-- Time created  : 14:33:40 02/15/2018                                     
-- Author        : Victor Arribas (victor.arribas@esat.kuleuven.be)                         
-- =============================================================================

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;

library work;
 use work.keccak_globals.all;
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity Keccak_f is
    Port ( 
    	s_in  : in   STD_LOGIC_VECTOR (25*N-1 downto 0);
        s_out : out  STD_LOGIC_VECTOR (25*N-1 downto 0)
	);
end Keccak_f;

architecture Behavioral of Keccak_f is
	
	type round_consts is array (0 to ROUNDS) of std_logic_vector(N-1 downto 0);

	type round_signs  is array (0 to ROUNDS) of k_state;


	constant BLOCK_BYTES : natural := N/8;

	signal RND_CNTS 	 : round_consts;

	component keccak_round_blocks 
		 port ( round_in     : in  k_state;
				  round_constant_signal    : in std_logic_vector(N-1 downto 0);
				  round_out    : out k_state);
	end component;

	signal round_in, round_out : round_signs;
	
	
begin

       RND_CNTS <=
                   ((X"0000000000000001"),
                        (X"0000000000008082"),
                        (X"800000000000808A"),
                        (X"8000000080008000"),
                        (X"000000000000808B"),
                        (X"0000000080000001"),
                        (X"8000000080008081"),
                        (X"8000000000008009"),
                        (X"000000000000008A"),
                        (X"0000000000000088"),
                        (X"0000000080008009"),
                        (X"000000008000000A"),
                        (X"000000008000808B"),
                        (X"800000000000008B"),
                        (X"8000000000008089"),
                        (X"8000000000008003"),
                        (X"8000000000008002"),
                        (X"8000000000000080"),
                        (X"000000000000800A"),
                        (X"800000008000000A"),
                        (X"8000000080008081"),
                        (X"8000000000008080"),
                        (X"0000000080000001"),
                        (X"8000000080008008"));
	
	--Load state or assign the previous step
	i1: for y in 0 to 4 generate
		i2: for x in 0 to 4 generate
			i3: for i in 0 to BLOCK_BYTES-1 generate
				i4: for z in 0 to 7 generate
			
					round_in(0)(y)(x)(8*i+z) <= s_in((N*5*(4-y))+(N*(4-x))+8*(BLOCK_BYTES-1-i)+z);
													
				end generate;	
			end generate;
		end generate;
	end generate;
		
	--Compute Keccak
	roundsss: for R in 0 to ROUNDS generate
		Kecc_round:	component keccak_round_blocks 
			port map( round_in => round_in(R), round_constant_signal => RND_CNTS(R), round_out => round_out(R));
	end generate;
	
	roundBuffers : for R in 1 to ROUNDS generate
		round_in(R) <= round_out(R-1);
	end generate;


	
	--Output
	o002: for x in 0 to 4 generate
		o02: for i in 0 to BLOCK_BYTES-1 generate
			o002: for z in 0 to 7 generate				
				s_out(N*(4-x)+8*(BLOCK_BYTES-1-i)+z+20*N)<= round_out(ROUNDS)(0)(x)(8*i+z) ;

				s_out(N*(4-x)+8*(BLOCK_BYTES-1-i)+z+15*N)<= round_out(ROUNDS)(1)(x)(8*i+z) ;

				s_out(N*(4-x)+8*(BLOCK_BYTES-1-i)+z+10*N)<= round_out(ROUNDS)(2)(x)(8*i+z)  ;

				s_out(N*(4-x)+8*(BLOCK_BYTES-1-i)+z+5*N)<= round_out(ROUNDS)(3)(x)(8*i+z)  ;

				s_out(N*(4-x)+8*(BLOCK_BYTES-1-i)+z)<= round_out(ROUNDS)(4)(x)(8*i+z)     ;
			end generate;
		end generate;
	end generate;
end Behavioral;
