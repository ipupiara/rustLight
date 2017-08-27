LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;

-- Top-level entity
ENTITY rustLight IS
	GENERIC ( amtTriacs : INTEGER := 4 ) ;
	PORT ( 	data : IN STD_LOGIC_VECTOR(9 DOWNTO 0) ;
				address : IN STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
				Clock, Reset, Strobe,ZeroPass, SwitchedOn : IN STD_LOGIC;
				triacTriggerPulses : OUT STD_LOGIC_VECTOR(amtTriacs - 1 DOWNTO 0) 
			 ) ;
END rustLight ;
ARCHITECTURE Behavior OF rustLight IS
--	SIGNAL DataReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
--	SIGNAL SwitchedOnReg, countersClockSig: STD_LOGIC;
	SIGNAL countersClockSig: STD_LOGIC;
	SIGNAL AddressReg : STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
	SIGNAL MuxOutputReg : STD_LOGIC_VECTOR( (amtTriacs * 11) -1 DOWNTO 0) ;	
	SIGNAL zeroPassAsync1, zeroPassAsync2: STD_LOGIC; 
--	SIGNAL strobeAsync1, strobeAsync2: STD_LOGIC;
	SIGNAL XSig : STD_LOGIC_VECTOR(10 DOWNTO 0) ;
	SIGNAL YSig : STD_LOGIC_VECTOR( (amtTriacs * 11) -1 DOWNTO 0) ;	
	
--	type triacDriverIF is 
--	RECORD
--		Dta : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
--		SwitchedOn : STD_LOGIC;
--	END RECORD;	
--	
--	type triacDriverIFArray is  array(NATURAL range <>) of triacDriverIF;
--	SIGNAL  TriacDriverInput : triacDriverIFArray(amtTriacs -1 downto 0);	
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
			 switchedOn, zeroPass    : in  STD_LOGIC;
			 Clock,Reset, CountersClock    : in  STD_LOGIC;
			 triacTriggerPulse : out STD_LOGIC;
			 testData : out std_LOGIC_VECTOR (10  downto 0)
		  );
	END COMPONENT;
	
	COMPONENT rustLightPLL_1 
		PORT
		(
			inclk0		: IN STD_LOGIC  := '0';
			c0		: OUT STD_LOGIC 
		);
	END COMPONENT;
	
	COMPONENT StrobeLatch 
	  generic (
		 BITS   : POSITIVE  := 8 );
	  port (
		  inData    : in  STD_LOGIC_VECTOR(BITS - 1 downto 0);
		  outData    : out  STD_LOGIC_VECTOR(BITS - 1 downto 0) ;
		  Clock, Strobe, Reset : in std_logic	
		 );
	END COMPONENT;
	
	component controllerInputChecker
	PORT
	(
		probe		: IN STD_LOGIC_VECTOR (43 DOWNTO 0);
		source		: OUT STD_LOGIC_VECTOR (0 DOWNTO 0)
	);
	end component;

	component inputProbe
		PORT
		(
			probe		: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
			source		: OUT STD_LOGIC_VECTOR (0 DOWNTO 0)
		);
	end component;
	
	component demuxInputChecker
	PORT
	(
		probe		: IN STD_LOGIC_VECTOR (12 DOWNTO 0);
		source		: OUT STD_LOGIC_VECTOR (0 DOWNTO 0)
	);
	end component;


	
	BEGIN
--		XSig <= DataReg & SwitchedOnReg;
		demuxer : DeMUX_1toX_N_bits
			GENERIC MAP (PORTS => amtTriacs , BITS => 11)
			PORT MAP (AddressReg,XSig, MuxOutputReg);
			
		dataLatch : strobeLatch	
			GENERIC MAP ( BITS => 11)
			PORT MAP (data & SwitchedOn,XSig,Clock, Strobe, Reset );
			
		addressLatch : strobeLatch
			GENERIC MAP ( BITS => 2)
			PORT MAP (address,AddressReg,Clock, Strobe, Reset );


		inputProbe_inst : inputProbe PORT MAP (
			probe	 => data & SwitchedOn & address  & Strobe & ZeroPass & Reset
--			, source	 => source_sig
		);
			
		demuxInputChecker_inst : demuxInputChecker PORT MAP (
			probe	 =>  XSig & AddressReg
--			,source	 => source_sig
		);
		
					
		rustLightControllerInputChecker :  controllerInputChecker PORT MAP (
			probe =>   MuxOutputReg
--				, source => source_sig
			);

			
		GEN_REG1: 
		for i1 in 0 to amtTriacs-1 generate 
--			GEN_REG3:	
--			if (i1 = 0) generate
--					rustLightControllerInputChecker :  controllerInputChecker PORT MAP (
--						probe =>   YSig
----						, source => source_sig
--						);
--						
--					REGX2 : triacDriver port map
--						 ( ignitionDelay => MuxOutputReg((((i1 + 1) * 11 ) - 2)  downto ((i1 * 11 )  )),
--						 switchedOn => MuxOutputReg((((i1 + 1) * 11 ) - 1)), zeroPassUp => zeroPassUpAsync2, 
--						 Clock => Clock, Reset => Reset,countersClock => countersClockSig,
--						 triacTriggerPulse => triacTriggerPulses(i1),
--						 testData => YSig);
--			end generate GEN_REG3;	
--			GEN_REG2: 
--				generate
					REGX1 : triacDriver port map
						 ( ignitionDelay => MuxOutputReg((((i1 + 1) * 11 ) - 2)  downto ((i1 * 11 )  )),
						 switchedOn => MuxOutputReg((((i1 + 1) * 11 ) - 1)), zeroPass => zeroPassAsync2, 
						 Clock => Clock, Reset => Reset,countersClock => countersClockSig,
						 triacTriggerPulse => triacTriggerPulses(i1)
--						 ,testData => YSig((((i1 + 1) * 11 ) - 1)  downto ((i1 * 11 )  )) 
						 );
--				end generate GEN_REG2; 
 
		end generate GEN_REG1;	
		
		
		pll_1: rustLightPLL_1
			PORT MAP (Clock, countersClockSig);
			
		PROCESS ( Reset, Clock )
		BEGIN
			IF Reset = '0' THEN
--				DataReg <= (OTHERS => '0'); 
--				addressReg  <= (OTHERS => '0'); 
--				SwitchedOnReg <= '0';
			ELSIF  (Clock'EVENT AND Clock = '1' ) THEN
				zeroPassAsync1 <= ZeroPass;
				zeroPassAsync2 <= zeroPassAsync1;
--				strobeAsync1 <= Strobe;
--				strobeAsync2 <=  strobeAsync1;
--				IF (strobeAsync2 = '1')  THEN
----					DataReg <= data; 
--					addressReg <= address;
--					SwitchedOnReg <= switchedOn;
--				END IF;	
			END IF ;
		END PROCESS ;

END Behavior ;

