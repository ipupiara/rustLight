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
	type triacStateType is (idle, triacDelayLoading, triacTriggerDelay, triacIgnitionDurationLoading, fireIgnitionTrigger, 
										fireBreakDurationLoading, fireBreak);
	SIGNAL triacState : triacStateType;
	SIGNAL IgnitionDelayReg, IgnitionDurationReg, fireBreakDurationReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL IgnitionDelayCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL IgnitionDurationCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL fireBreakDurationCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL SwitchedOnReg, ZeroPassUpReg   : STD_LOGIC;
	SIGNAL sloadIgnitionDelaySig, cntEnaIgnitionDelaySig, equalIgnitionDelaySig : std_LOGIC;
	SIGNAL sloadIgnitionDurationSig, cntEnaIgnitionDurationSig, equalIgnitionDurationSig : std_LOGIC;
	SIGNAL sloadFireBreakDurationSig, cntEnaFireBreakDurationSig, equalFireBreakDurationSig: std_LOGIC;

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
		IgnitionDurationReg <=  "1111111100";
		fireBreakDurationReg <= "0000000100";
		SwitchedOnReg <= switchedOn; 
		ZeroPassUpReg <= zeroPassUp;
--		sloadSig <= '0';
		PROCESS ( Reset, Clock )	
			procedure entryIdle is
			begin
				triacTriggerPulse <= '0';
				triacState <= idle;
				sloadIgnitionDelaySig <= '0';
				cntEnaIgnitionDelaySig  <= '0';
				sloadIgnitionDurationSig <= '0';
				cntEnaIgnitionDurationSig <= '0';
				sloadFireBreakDurationSig  <= '0';
				cntEnaFireBreakDurationSig  <= '0';
			end procedure entryIdle;
		BEGIN
			IF ( Reset = '1' ) THEN
				entryIdle;
			ELSIF ( Clock'EVENT AND Clock = '1' ) THEN
				IF ((SwitchedOnReg = '0') OR (ZeroPassUpReg = '0')) THEN 
					entryIdle; 
				END IF;
				IF ((triacState = idle) AND (ZeroPassUpReg = '1')) THEN
					sloadIgnitionDelaySig <= '1';
					triacState <= triacDelayLoading;
				ELSIF (triacState = triacDelayLoading) THEN
						sloadIgnitionDelaySig <= '0';
						cntEnaIgnitionDelaySig <= '1';
						triacState <= triacTriggerDelay;
				END IF;
				IF (((triacState = triacTriggerDelay) AND (equalIgnitionDelaySig = '1'))
						OR ((triacState = fireBreak ) AND (equalFireBreakDurationSig = '1')) )   THEN
					cntEnaIgnitionDelaySig <= '0';
					cntEnaFireBreakDurationSig <= '0';
					sloadIgnitionDurationSig <= '1';
					triacState <= triacIgnitionDurationLoading;
				ELSIF (triacState = triacIgnitionDurationLoading) THEN
					sloadIgnitionDurationSig <= '0';
					cntEnaIgnitionDurationSig <= '1';
					triacState <= fireIgnitionTrigger;
					triacTriggerPulse <= '1';
				END IF;
				IF ((triacState = fireIgnitionTrigger)  AND (equalIgnitionDurationSig = '1' ))  THEN
					triacTriggerPulse <= '0';
					cntEnaIgnitionDurationSig <= '0';
					sloadFireBreakDurationSig <= '1';
					triacState <= fireBreakDurationLoading;
				ELSIF (triacState= fireBreakDurationLoading ) THEN
					sloadFireBreakDurationSig <= '0';
					cntEnaFireBreakDurationSig <= '1';
					triacState <= fireBreak;
				END IF;
				
			END IF;	
		END PROCESS ;
		rustLightIgnitionDelayCounter : rustLightCounter PORT MAP (
			clock	 => countersClock,
			cnt_en => cntEnaIgnitionDelaySig,
			data	 => IgnitionDelayReg,
			sload	 => sloadIgnitionDelaySig,
			q	 => IgnitionDelayCounterReg
		);
		rustLightIgnitionDelayCompare : rustLightCompare_1 PORT MAP (
			dataa	 => IgnitionDelayCounterReg,
			aeb	 => equalIgnitionDelaySig
		);
		rustLightIgnitionDurationCounter : rustLightCounter PORT MAP (
			clock	 => Clock,
			cnt_en => cntEnaIgnitionDurationSig,
			data	 => IgnitionDurationReg,
			sload	 => sloadIgnitionDurationSig,
			q	 => IgnitionDurationCounterReg
		);
		rustLightIgnitionDurationCompare : rustLightCompare_1 PORT MAP (
			dataa	 => IgnitionDurationCounterReg,
			aeb	 => equalIgnitionDurationSig
		);
		
		rustLightFireBreakDurationCounter : rustLightCounter PORT MAP (
			clock	 => countersClock,
			cnt_en => cntEnaFireBreakDurationSig,
			data	 => fireBreakDurationReg,
			sload	 => sloadFireBreakDurationSig,
			q	 => fireBreakDurationCounterReg
		);
		rustLightFireBreakDurationCompare : rustLightCompare_1 PORT MAP (
			dataa	 => fireBreakDurationCounterReg,
			aeb	 => equalFireBreakDurationSig
		);
		
		

end;
