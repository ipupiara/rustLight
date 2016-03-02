LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;


--  basic entities
--  both copyed from the internet

--  register

--ENTITY regn IS
--	GENERIC ( N : INTEGER := 16 ) ;
--	PORT ( D : IN STD_LOGIC_VECTOR(N−1 DOWNTO 0) ;
--		Resetn, Clock, Enable : IN STD_LOGIC ;
--		Q : OUT STD_LOGIC_VECTOR(N−1 DOWNTO 0) ) ;
--END regn ;
--ARCHITECTURE Behavior OF regn IS
--BEGIN
--	PROCESS ( Resetn, Clock )
--	BEGIN
--		IF (Resetn = ’0’) THEN
--			Q <=(OTHERS=> ’0’) ;
--		ELSIF ( (Clock’EVENT AND Clock = ’1’ ) AND (Enable = 1) ) THEN
--			Q <= D ;
--		END IF ;
--	END PROCESS ;
--END Behavior ;
--

-- demuxer

entity DeMUX_1toX_N_bits is
  generic (
    PORTS  : POSITIVE  := 4;
    BITS   : POSITIVE  := 8 );
  port (
    sel  : in  STD_LOGIC_VECTOR(integer(ceil(log2(real(PORTS)))) - 1 downto 0);
     X    : in  STD_LOGIC_VECTOR(BITS - 1 downto 0);
    Y    : out  STD_LOGIC_VECTOR((BITS * PORTS) - 1 downto 0)  
    );
end;

architecture rtl of DeMUX_1toX_N_bits is
  type T_SLVV is array(NATURAL range <>) of STD_LOGIC_VECTOR(BITS - 1 downto 0);
  signal mux_out : T_SLVV(PORTS - 1 downto 0);
  begin
  gen : for i in 0 to PORTS - 1 generate
    -- connect Output FLAT VECTOR to the correct T_SLVV lines
     Y(((i + 1) * BITS) - 1 downto (i * BITS)) <=  mux_out(i);
  end generate;

  mux_out(to_integer(unsigned(sel))) <= X;
end;
