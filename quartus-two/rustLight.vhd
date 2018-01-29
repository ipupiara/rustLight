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
				triacTriggerPulses : OUT STD_LOGIC_VECTOR(amtTriacs - 1 DOWNTO 0) ;
				countersClockOut: OUT STD_LOGIC
			 ) ;
END rustLight ;
ARCHITECTURE Behavior OF rustLight IS
--	SIGNAL DataReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
--	SIGNAL SwitchedOnReg, countersClockSig: STD_LOGIC;
	SIGNAL countersClockSig: STD_LOGIC;
	SIGNAL countersClockGlobalSig : STD_LOGIC;
-- SIGNAL countersClockOutSig : STD_LOGIC;
	SIGNAL AddressReg : STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
	SIGNAL MuxOutputReg : STD_LOGIC_VECTOR( (amtTriacs * 11) -1 DOWNTO 0) ;	
	SIGNAL zeroPassAsync1, zeroPassAsync2: STD_LOGIC; 
--	SIGNAL strobeAsync1, strobeAsync2: STD_LOGIC;
	SIGNAL XSig : STD_LOGIC_VECTOR(10 DOWNTO 0) ;
	SIGNAL YSig : STD_LOGIC_VECTOR( (amtTriacs * 11) -1 DOWNTO 0) ;	
	
	COMPONENT DeMUX_1toX_N_bits
		generic (
			 PORTS  : POSITIVE  := 4;
			 BITS   : POSITIVE  := 8 );
		port (
			 sel  : in  STD_LOGIC_VECTOR(integer(ceil(log2(real(PORTS)))) - 1 downto 0);
			 X    : in  STD_LOGIC_VECTOR(BITS - 1 downto 0);
			 Y    : out  STD_LOGIC_VECTOR((BITS * PORTS) - 1 downto 0);
			 Clock, Reset : IN STD_LOGIC
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
	
	component clkctrl_1
	PORT
	(
		inclk		: IN STD_LOGIC ;
		outclk		: OUT STD_LOGIC 
	);
	end component;
	
--	component clkCtrlOut
--	PORT
--	(
--		inclk		: IN STD_LOGIC ;
--		outclk		: OUT STD_LOGIC 
--	);
--	end component;
	
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

--	component rustLightCounter
--		PORT
--		(
--			aclr		: IN STD_LOGIC ;
--			clock		: IN STD_LOGIC ;
--			cnt_en		: IN STD_LOGIC ;
--			q		: OUT STD_LOGIC_VECTOR (9 DOWNTO 0)	
--		);
--	end component;
--	component rustLightCompare_1
--		PORT
--		(
--			dataa		: IN STD_LOGIC_VECTOR (9 DOWNTO 0);
--			datab		: IN STD_LOGIC_VECTOR (9 DOWNTO 0);
--			ageb		: OUT STD_LOGIC 
--		);
--	end component;
--
--	component rl_sync_counter
--		PORT
--		(
--			clock		: IN STD_LOGIC ;
--			cnt_en		: IN STD_LOGIC ;
--			sclr		: IN STD_LOGIC ;
--			q		: OUT STD_LOGIC_VECTOR (13 DOWNTO 0)
--		);
--	end component;
--	
--	component rl_sync_Compare
--	PORT
--	(
--		dataa		: IN STD_LOGIC_VECTOR (13 DOWNTO 0);
--		datab		: IN STD_LOGIC_VECTOR (13 DOWNTO 0);
--		ageb		: OUT STD_LOGIC 
--	);
--	end component;
--	
--	
--
--
--component counterChecker
--	PORT
--	(
--		probe		: IN STD_LOGIC_VECTOR (14 DOWNTO 0);
--		source		: OUT STD_LOGIC_VECTOR (16 DOWNTO 0)
--	);
--end component;
--
--
--	SIGNAL IgnitionDelayReg : STD_LOGIC_VECTOR(13 DOWNTO 0) ;
--	SIGNAL IgnitionDelayCounterReg : STD_LOGIC_VECTOR(13 DOWNTO 0) ;
--	SIGNAL aclrIgnitionDelaySig, equalIgnitionDelaySig, equalIgnitionDelaySigA1, equalIgnitionDelaySigA2 : std_LOGIC;
--	SIGNAL cnt_En_triacDelay, testSig, countersClock, clk : std_Logic;
--	SIGNAL cnt_En_fireCounter, sclr_fireCounter, ageb_fireCounter : std_LOGIC;
--	SIGNAL q_fireCounter, dataa_fireCounter : STD_LOGIC_VECTOR (13 DOWNTO 0);
--	SIGNAL sourceSig : STD_LOGIC_VECTOR (16 downto 0);
--	
	BEGIN
--		XSig <= DataReg & SwitchedOnReg;
		countersClockOut <= countersClockGlobalSig;
		demuxer : DeMUX_1toX_N_bits
			GENERIC MAP (PORTS => amtTriacs , BITS => 11)
			PORT MAP (AddressReg,XSig, MuxOutputReg, Clock, Reset);
			
		dataLatch : strobeLatch	
			GENERIC MAP ( BITS => 11)
			PORT MAP (data & SwitchedOn,XSig,Clock, Strobe, Reset );
			
		addressLatch : strobeLatch
			GENERIC MAP ( BITS => integer(ceil(log2(real(amtTriacs)))))
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
			probe =>   YSig
--				, source => source_sig
		);

			
		GEN_REG0: 
		for i1 in 0 to 0 generate 
					REGX1 : triacDriver port map
						 ( ignitionDelay => MuxOutputReg((((i1 + 1) * 11 ) - 2)  downto ((i1 * 11 )  )),
						 switchedOn => MuxOutputReg((((i1 + 1) * 11 ) - 1)), zeroPass => zeroPassAsync2, 
						 Clock => Clock, Reset => Reset,countersClock => countersClockGlobalSig,
						 triacTriggerPulse => triacTriggerPulses(i1),
						 testData => YSig((((i1 + 1) * 11 ) - 1)  downto ((i1 * 11 )  )) 
						 );
		end generate GEN_REG0;	
		GEN_REG1: 
		for i1 in 1 to amtTriacs-1 generate 
					REGX1 : triacDriver port map
						 ( ignitionDelay => MuxOutputReg((((i1 + 1) * 11 ) - 2)  downto ((i1 * 11 )  )),
						 switchedOn => MuxOutputReg((((i1 + 1) * 11 ) - 1)), zeroPass => zeroPassAsync2, 
						 Clock => Clock, Reset => Reset,countersClock => countersClockGlobalSig,
						 triacTriggerPulse => triacTriggerPulses(i1),
						 testData => YSig((((i1 + 1) * 11 ) - 1)  downto ((i1 * 11 )  )) 
						 );
		end generate GEN_REG1;	
		
		
		pll_1: rustLightPLL_1
			PORT MAP (Clock, countersClockSig);
			
		clkctrl_1Impl: clkctrl_1
			PORT Map (countersClockSig,countersClockGlobalSig);

			
		PROCESS ( Reset, Clock )
		BEGIN
			IF Reset = '0' THEN

			ELSIF  (Clock'EVENT AND Clock = '1' ) THEN
				zeroPassAsync1 <= ZeroPass;
				zeroPassAsync2 <= zeroPassAsync1;

			END IF ;
		END PROCESS ;
		
--		rustLightIgnitionDelayCounter : rl_sync_counter PORT MAP (
--			clock	 => countersClock,
--			sclr	 => aclrIgnitionDelaySig,
--			cnt_en => cnt_En_triacDelay,
--			q	 => IgnitionDelayCounterReg
--		);
--		rustLightIgnitionDelayCompare : rl_sync_Compare PORT MAP (
--			dataa	 => IgnitionDelayCounterReg,
--			datab  => IgnitionDelayReg,
--			ageb   => equalIgnitionDelaySig
--		);
--		
----		IgnitionDelayReg <= "1100000000";
----		sourceSig <= countersClock & aclrIgnitionDelaySig & cnt_En_triacDelay & IgnitionDelayReg;
--		
--			IgnitionDelayReg <= sourceSig (13 downto 0);
--			countersClock <= sourceSig(16);
--			aclrIgnitionDelaySig <= sourceSig(15);
--			cnt_En_triacDelay <= sourceSig(14);
--		
--		counterCheckerInst : counterChecker PORT MAP (
--			probe =>  equalIgnitionDelaySig &  IgnitionDelayCounterReg ,
--			source => sourceSig
------			source => countersClock & aclrIgnitionDelaySig & cnt_En_triacDelay
--		);
--		
--		rustLightFireCounter : rl_sync_counter PORT MAP (
--			clock => clk,
--			cnt_en => cnt_En_fireCounter,
--			sclr =>  sclr_fireCounter ,
--			q =>  q_fireCounter
--		);
--		rustLightFireCompare : rl_sync_Compare PORT MAP (
--			dataa => q_fireCounter,
--			datab => dataa_fireCounter,
--			ageb => ageb_fireCounter
--		);

END Behavior ;

