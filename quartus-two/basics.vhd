LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;


--  basic entities
--  copyed from the internet

-- demuxer

LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;


entity DeMUX_1toX_N_bits is
  generic (
    PORTS  : POSITIVE  := 4;
    BITS   : POSITIVE  := 8 );
  port (
		sel  : in  STD_LOGIC_VECTOR(integer(ceil(log2(real(PORTS)))) - 1 downto 0);
		X    : in  STD_LOGIC_VECTOR(BITS - 1 downto 0);
		Y    : out  STD_LOGIC_VECTOR((BITS * PORTS) - 1 downto 0);
		Clock, Reset : IN STD_LOGIC
    );
end;

architecture rtl of DeMUX_1toX_N_bits is
  type T_SLVV is array(NATURAL range <>) of STD_LOGIC_VECTOR(BITS - 1 downto 0);
  signal mux_out : T_SLVV(PORTS - 1 downto 0);
  begin
  gen : for i in 0 to PORTS - 1 generate
     Y(((i + 1) * BITS) - 1 downto (i * BITS)) <=  mux_out(i);
  end generate;
  
  	PROCESS ( Reset, Clock )
		BEGIN
			IF Reset = '0' THEN
--				DataReg <= (OTHERS => '0'); 
--				addressReg  <= (OTHERS => '0'); 
--				SwitchedOnReg <= '0';
			ELSIF  (Clock'EVENT AND Clock = '1' ) THEN
				mux_out(to_integer(unsigned(sel))) <= X;
			END IF ;
		END PROCESS ;


--  mux_out(to_integer(unsigned(sel))) <= X;
end;
