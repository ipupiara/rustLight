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


entity StrobeLatch is
  generic (
    BITS   : POSITIVE  := 8 );
  port (
     inData    : in  STD_LOGIC_VECTOR(BITS - 1 downto 0);
     outData    : out  STD_LOGIC_VECTOR(BITS - 1 downto 0) ;
	  Clock, Strobe, Reset : in std_logic	
    );
end;

architecture rtl of StrobeLatch is
	SIGNAL strobeAsync1, strobeAsync2: STD_LOGIC;
	
	BEGIN	
	PROCESS ( Reset, Clock )
	BEGIN
		IF Reset = '0' THEN
			strobeAsync1 <= '0';
			strobeAsync2  <= '0';
		ELSIF  (Clock'EVENT AND Clock = '1' ) THEN
			strobeAsync1 <= Strobe;
			strobeAsync2 <=  strobeAsync1;
			IF (strobeAsync2 = '1')  THEN
				outData <= inData; 
			END IF;
		END IF ;
	END PROCESS ;
   
end;
