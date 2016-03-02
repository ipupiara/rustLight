LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;


--  basic entities
--  both copyed from the internet

--  register

ENTITY signalDelayer is
	PORT (
				Reset, Clock,Enable, sigIn : IN STD_LOGIC;
				sigOut: OUT STD_LOGIC
			);
END;
ARCHITECTURE behave OF signalDelayer is
TYPE signalDelayStateType is (disabled, idle, receivedFirstClock);
SIGNAL signalDelayState : signalDelayStateType;
BEGIN
	PROCESS (  Clock )
	BEGIN
		IF (Reset = '0') THEN
			signalDelayState <= idle;
			sigOut <= '0';
		ELSIF (Clock'EVENT AND Clock = '1') THEN
			IF (Enable = '0') THEN	
				signalDelayState <= disabled;
			ELSE	
				IF (signalDelayState = disabled)  THEN
					signalDelayState <= idle;
				END IF;	
				IF (signalDelayState = idle) THEN
						signalDelayState <= receivedFirstClock;
				ELSIF (signalDelayState = receivedFirstClock) THEN
						sigOut <= sigIn;
						signalDelayState <= idle;
				END IF;
			END IF; 
		END IF;
	END PROCESS ;
	
--	PROCESS ( Enable)
--	BEGIN
--		if (Enable = '0') then
--			signalDelayState <= disabled;
--		ELSIF	((Enable = '1') AND (signalDelayState = disabled) ) THEN
--			signalDelayState <= idle;
--		END IF;	
--	END PROCESS; 

END behave;

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
