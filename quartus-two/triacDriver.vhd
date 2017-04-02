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
	 triacTriggerPulse : out STD_LOGIC;
	 testData : out std_LOGIC_VECTOR (10  downto 0)
  );
end;

architecture driverJob of triacDriver is
	type triacStateType is (idle,  triacTriggerDelay, fireIgnitionTrigger, fireBreak);
	SIGNAL triacState : triacStateType;
	SIGNAL IgnitionDelayReg, IgnitionDurationReg, fireBreakDurationReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL IgnitionDelayCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL IgnitionDurationCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL fireBreakDurationCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL SwitchedOnReg, ZeroPassUpReg   : STD_LOGIC;
	SIGNAL sclrIgnitionDelaySig, cntEnaIgnitionDelaySig, equalIgnitionDelaySig : std_LOGIC;
	SIGNAL sclrIgnitionDurationSig, cntEnaIgnitionDurationSig, equalIgnitionDurationSig : std_LOGIC;
	SIGNAL sclrFireBreakDurationSig, cntEnaFireBreakDurationSig, equalFireBreakDurationSig: std_LOGIC;
--	SIGNAL source_sig : STD_LOGIC_VECTOR (0 DOWNTO 0);
--	SIGNAL probeReg : STD_LOGIC_VECTOR (10 DOWNTO 0);

component rustLightCounter
	PORT
	(
		clock		: IN STD_LOGIC ;
		cnt_en		: IN STD_LOGIC ;
		q		: OUT STD_LOGIC_VECTOR (9 DOWNTO 0);
		sclr		: IN STD_LOGIC 
	);
end component;
component rustLightCompare_1
	PORT
	(
		aleb		: OUT STD_LOGIC ;
		dataa		: IN STD_LOGIC_VECTOR (9 DOWNTO 0);
		datab		: IN STD_LOGIC_VECTOR (9 DOWNTO 0)
	);
end component;

  begin
  		IgnitionDelayReg <= ignitionDelay; 
		IgnitionDurationReg <=  "1111111100";
		fireBreakDurationReg <= "0000000100";
		SwitchedOnReg <= switchedOn; 
		ZeroPassUpReg <= zeroPassUp;
--		ProbeReg <=  IgnitionDelayReg & SwitchedOnReg;
			testData <= 	IgnitionDelayReg & SwitchedOnReg;
		PROCESS ( Reset, Clock )	
			procedure entryIdle is
			begin
				triacTriggerPulse <= '0';
				triacState <= idle;
				sclrIgnitionDelaySig <= '0';
				cntEnaIgnitionDelaySig  <= '0';
				sclrIgnitionDurationSig <= '0';
				cntEnaIgnitionDurationSig <= '0';
				sclrFireBreakDurationSig  <= '0';
				cntEnaFireBreakDurationSig  <= '0';
			end procedure entryIdle;
		BEGIN
			IF ( Reset = '1' ) THEN
				entryIdle;
			ELSIF ( Clock'EVENT AND Clock = '1' ) THEN
				IF ((SwitchedOnReg = '0') OR (ZeroPassUpReg = '0')) THEN 
					entryIdle; 
				ELSE
					sclrFireBreakDurationSig <= '0';
					sclrIgnitionDelaySig <= '0';
					sclrIgnitionDurationSig <= '0';
					IF ((triacState = idle) AND (ZeroPassUpReg = '1')) THEN
						sclrIgnitionDelaySig <= '1';
						cntEnaIgnitionDelaySig <= '1';
						triacState <= triacTriggerDelay;
					END IF;
					IF (((triacState = triacTriggerDelay) AND (equalIgnitionDelaySig = '1'))
							OR ((triacState = fireBreak ) AND (equalFireBreakDurationSig = '1')) )   THEN
						cntEnaIgnitionDelaySig <= '0';
						cntEnaFireBreakDurationSig <= '0';
						sclrIgnitionDurationSig <= '1';
						cntEnaIgnitionDurationSig <= '1';
						triacState <= fireIgnitionTrigger;
						triacTriggerPulse <= '1';
					END IF;
					IF ((triacState = fireIgnitionTrigger)  AND (equalIgnitionDurationSig = '1' ))  THEN
						triacTriggerPulse <= '0';
						cntEnaIgnitionDurationSig <= '0';
						sclrFireBreakDurationSig <= '1';
						cntEnaFireBreakDurationSig <= '1';
						triacState <= fireBreak;
					END IF;
				END IF;   -- not idle	
			END IF;   -- not reset
		END PROCESS ;
		rustLightIgnitionDelayCounter : rustLightCounter PORT MAP (
			clock	 => countersClock,
			cnt_en => cntEnaIgnitionDelaySig,
			sclr	 => sclrIgnitionDelaySig,
			q	 => IgnitionDelayCounterReg
		);
		rustLightIgnitionDelayCompare : rustLightCompare_1 PORT MAP (
			aleb   => equalIgnitionDelaySig,
			dataa	 => IgnitionDelayCounterReg,
			datab  => IgnitionDelayReg
		);
		rustLightIgnitionDurationCounter : rustLightCounter PORT MAP (
			clock	 => Clock,
			cnt_en => cntEnaIgnitionDurationSig,
			sclr	 => sclrIgnitionDurationSig,
			q	 => IgnitionDurationCounterReg
		);
		rustLightIgnitionDurationCompare : rustLightCompare_1 PORT MAP (
			aleb	 => equalIgnitionDurationSig,
			dataa	 => IgnitionDurationCounterReg,
			datab	 => IgnitionDurationReg
		);
		
		rustLightFireBreakDurationCounter : rustLightCounter PORT MAP (
			clock	 => countersClock,
			cnt_en => cntEnaFireBreakDurationSig,
			sclr	 => sclrFireBreakDurationSig,
			q	 => fireBreakDurationCounterReg
		);
		rustLightFireBreakDurationCompare : rustLightCompare_1 PORT MAP (
			aleb	 => equalFireBreakDurationSig,
			dataa	 => fireBreakDurationCounterReg,
			datab	 => fireBreakDurationReg
		);
end;
