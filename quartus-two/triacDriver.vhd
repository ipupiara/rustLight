LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;

-- demuxer

entity triacDriver is
  port (
    ignitionDelay  : in  STD_LOGIC_VECTOR(9 downto 0);
    switchedOn, zeroPassUp    : in  STD_LOGIC;
	 Clock,Reset, CountersClock  : in  STD_LOGIC;
	 triacTriggerPulse : out STD_LOGIC
  );
end;

architecture driverJob of triacDriver is
	type triacStateType is (idle, triacTriggerDelay, fireTrigger, fireDelay);
	SIGNAL triacState : triacStateType;
	SIGNAL IgnitionDelayReg, IgnitionBreakReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL IgnitionDelayCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL IgnitionBreakCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL SwitchedOnReg, ZeroPassUpReg   : STD_LOGIC;
	SIGNAL sloadIgnitionDelaySig, cntEnaIgnitionDelaySig, equalIgnitionDelaySig : std_LOGIC;
	SIGNAL sloadIgnitionBreakSig, cntEnaIgnitionBreakSig, equalIgnitionBreakSig : std_LOGIC;

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
component rustLightCompare_1
	PORT
	(
		dataa		: IN STD_LOGIC_VECTOR (9 DOWNTO 0);
		aeb		: OUT STD_LOGIC 
	);
end component;
  begin
  		IgnitionDelayReg <= ignitionDelay; 
		IgnitionBreakReg <= "1111111100";
		SwitchedOnReg <= switchedOn; 
		ZeroPassUpReg <= zeroPassUp;
--		sloadSig <= '0';
		PROCESS ( Reset, Clock )
		BEGIN
			IF ( Reset = '1' ) THEN
				triacTriggerPulse <= '0';
				triacState <= idle;
				sloadIgnitionDelaySig <= '0';
				cntEnaIgnitionDelaySig  <= '0';
			ELSIF ( Clock'EVENT AND Clock = '1' ) THEN
--				IgnitionDelayReg <= ignitionDelay; 
				sloadIgnitionDelaySig <= '0';
				cntEnaIgnitionDelaySig <= '0';
				--  trivial Code, so that all lines are used and not optimized away by the synthesis/fitter
				IF (ZeroPassUpReg = '0') THEN
					triacTriggerPulse <= '0';
				ELSIF ( (( to_integer(unsigned(IgnitionDelayReg  ))) > 0 ) AND (SwitchedOnReg = '1'))  THEN
					triacTriggerPulse <= '1';
				END IF;			
			END IF ;
		END PROCESS ;
		rustLightIgnitionDelayCounter_inst : rustLightCounter PORT MAP (
			clock	 => countersClock,
			cnt_en => cntEnaIgnitionDelaySig,
			data	 => IgnitionDelayReg,
			sload	 => sloadIgnitionDelaySig,
			q	 => IgnitionDelayCounterReg
		);
		rustLightIgnitionDelayCompare_inst : rustLightCompare_1 PORT MAP (
			dataa	 => IgnitionDelayCounterReg,
			aeb	 => equalIgnitionDelaySig
		);
		rustLightIgnitionBreakCounter_inst : rustLightCounter PORT MAP (
			clock	 => Clock,
			cnt_en => cntEnaIgnitionBreakSig,
			data	 => IgnitionBreakReg,
			sload	 => sloadIgnitionBreakSig,
			q	 => IgnitionBreakCounterReg
		);
		rustLightIgnitionBreakCompare_inst : rustLightCompare_1 PORT MAP (
			dataa	 => IgnitionBreakCounterReg,
			aeb	 => equalIgnitionBreakSig
		);
end;
