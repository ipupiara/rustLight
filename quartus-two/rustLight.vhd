LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;

-- Top-level entity
ENTITY rustLight IS
	GENERIC ( amtTriacs : INTEGER := 4 ) ;
	PORT ( 	data : IN STD_LOGIC_VECTOR(9 DOWNTO 0) ;
				address : IN STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
				Clock, Reset, Strobe,zeroPassUp1, zeroPassUp2, SwitchedOn : IN STD_LOGIC;
				triacTriggerPulses : OUT STD_LOGIC_VECTOR(amtTriacs - 1 DOWNTO 0) 
			 ) ;
END rustLight ;
ARCHITECTURE Behavior OF rustLight IS
	SIGNAL DataReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL SwitchedOnReg, countersClockSig: STD_LOGIC;
	SIGNAL AddressReg : STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
	SIGNAL MuxOutputReg : STD_LOGIC_VECTOR( (amtTriacs * 11) -1 DOWNTO 0) ;	
	SIGNAL zeroPassUpAsync1, zeroPassUpAsync2: STD_LOGIC; 
	SIGNAL strobeAsync1, strobeAsync2: STD_LOGIC;
	
	type triacDriverIF is 
	RECORD
		Dta : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
		SwitchedOn : STD_LOGIC;
	END RECORD;	
	
	type triacDriverIFArray is  array(NATURAL range <>) of triacDriverIF;
	SIGNAL  TriacDriverInput : triacDriverIFArray(amtTriacs -1 downto 0);	
	COMPONENT DeMUX_1toX_N_bits
		generic (
			 PORTS  : POSITIVE  := 4;
			 BITS   : POSITIVE  := 8 );
		port (
			 sel  : in  STD_LOGIC_VECTOR(integer(ceil(log2(real(PORTS)))) - 1 downto 0);
			 X    : in  STD_LOGIC_VECTOR(BITS - 1 downto 0);
			 Y    : out  STD_LOGIC_VECTOR((BITS * PORTS) - 1 downto 0)  
		);	
	END COMPONENT;
	
	COMPONENT triacDriver 
		port (
			 ignitionDelay  : in  STD_LOGIC_VECTOR(9 downto 0);
			 switchedOn, zeroPassUp    : in  STD_LOGIC;
			 Clock,Reset, CountersClock    : in  STD_LOGIC;
			 triacTriggerPulse : out STD_LOGIC
		  );
	END COMPONENT;
	
	COMPONENT rustLightPLL_1 
		PORT
		(
			inclk0		: IN STD_LOGIC  := '0';
			c0		: OUT STD_LOGIC 
		);
	END COMPONENT;

	
	BEGIN
--		StrobeReg <= Strobe;
			
		demuxer : DeMUX_1toX_N_bits
			GENERIC MAP (PORTS => amtTriacs , BITS => 11)
			PORT MAP (AddressReg,DataReg & SwitchedOnReg, MuxOutputReg);
			
		GEN_REG: 
		for i1 in 0 to amtTriacs-1 generate
			REGX : triacDriver port map
				(MuxOutputReg((((i1 + 1) * 11 ) - 2)  downto ((i1 * 11 )  )),
				 MuxOutputReg((((i1 + 1) * 11 ) - 1)), zeroPassUpAsync2, Clock, Reset,countersClockSig,
				 triacTriggerPulses(i1));
		end generate GEN_REG;	
		
		pll_1: rustLightPLL_1
			PORT MAP (Clock, countersClockSig);
			
		PROCESS ( Reset, Clock )
		BEGIN
			IF Reset = '1' THEN
				DataReg <= (OTHERS => '0'); 
				addressReg  <= (OTHERS => '0'); 
				SwitchedOnReg <= '0';
			ELSIF  (Clock'EVENT AND Clock = '1' ) THEN
				zeroPassUpAsync1 <= zeroPassUp1 OR zeroPassUp2;
				zeroPassUpAsync2 <= zeroPassUpAsync1;
				strobeAsync1 <= Strobe;
				strobeAsync2 <=  strobeAsync1;
				IF (strobeAsync2 = '1')  THEN
					DataReg <= data; 
					addressReg <= address;
					SwitchedOnReg <= switchedOn; 
				END IF;
			END IF ;
		END PROCESS ;

END Behavior ;

