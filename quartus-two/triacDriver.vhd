LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;

-- demuxer

entity triacDriver is
  port (
    ignitionDelay  : in  STD_LOGIC_VECTOR(9 downto 0);
    switchOnOff, zeroPassDown, zeroPassUp    : in  STD_LOGIC;
	 Clock,Reset, CountersClock  : in  STD_LOGIC;
	 triacTriggerPulse : out STD_LOGIC
  );
end;

architecture driverJob of triacDriver is
	type triacStateType is (idle, triacTriggerDelay, fireTrigger, fireDelay);
	SIGNAL triacState : triacStateType;
	SIGNAL IgnitionDelayReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL IgnitionDelayCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL SwitchOnOffReg, ZeroPassDownReg, ZeroPassUpReg   : STD_LOGIC;
	SIGNAL sourceReg : STD_LOGIC_VECTOR (1 DOWNTO 0);
	SIGNAL countersClockSig: STD_LOGIC;
	SIGNAL sloadSig: STD_LOGIC;
	SIGNAL cntEnaSig : std_LOGIC;
component testPluginTriacDriverDelay
	PORT
	(
		probe		: IN STD_LOGIC_VECTOR (9 DOWNTO 0);
		source		: OUT STD_LOGIC_VECTOR (1 DOWNTO 0)
	);
end component;
component rustLightCounter
	PORT
	(
		clock		: IN STD_LOGIC ;
		cnt_en		: IN STD_LOGIC ;
		data		: IN STD_LOGIC_VECTOR (9 DOWNTO 0);
		sload		: IN STD_LOGIC ;
		q		: OUT STD_LOGIC_VECTOR (9 DOWNTO 0)
	);
end component;
  begin
  		IgnitionDelayReg <= ignitionDelay; 
		countersClockSig <= countersClock;
		IgnitionDelayCounterReg <= (OTHERS => '0'); 
--		sloadSig <= '0';
		PROCESS ( Reset, Clock )
		BEGIN
			IF ( Reset = '1' ) THEN
--				IgnitionDelayReg <= (OTHERS => '0'); 
				SwitchOnOffReg <= '0';
				ZeroPassDownReg <= '0';
				ZeroPassUpReg <= '0';
				triacTriggerPulse <= '0';
				triacState <= idle;
				sloadSig <= '1';
				cntEnaSig  <= '1';
			ELSIF ( Clock'EVENT AND Clock = '1' ) THEN
--				IgnitionDelayReg <= ignitionDelay; 
				SwitchOnOffReg <= switchOnOff; 
				ZeroPassDownReg <= zeroPassDown;
				ZeroPassUpReg <= zeroPassUp;
				sloadSig <= '0';
				cntEnaSig <= '0';
				--  trivial Code, so that all lines are used and not optimized away by the synthesis/fitter
				IF (ZeroPassDownReg = '1') THEN
					triacTriggerPulse <= '0';
				ELSIF ( (( to_integer(unsigned(IgnitionDelayReg  ))) > 0 ) AND (SwitchOnOffReg = '1'))  THEN
					triacTriggerPulse <= '1';
				END IF;			
			END IF ;
		END PROCESS ;
		PROCESS ( CountersClock)
		BEGIN
			IF (CountersClock'EVENT AND CountersClock = '1' ) THEN
				--countersClockSig  <=  CountersClock;
				--If ( to_integer(unsigned(IgnitionDelayReg  )) > 0) THEN
					-- IgnitionDelayReg <= IgnitionDelayReg - minusReg;
				--END IF;	
			END IF;
		END PROCESS;
		testPluginTriacDriverDelay_inst : testPluginTriacDriverDelay 
			PORT MAP (
				probe	 => IgnitionDelayReg ,
				source	 => sourceReg
		);
		rustLightCounter_inst : rustLightCounter PORT MAP (
			clock	 => countersClockSig,
			cnt_en => cntEnaSig,
			data	 => IgnitionDelayReg,
			sload	 => sloadSig,
			q	 => IgnitionDelayCounterReg
		);
end;
