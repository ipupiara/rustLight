LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;

-- demuxer

entity triacDriver is
  port (
    ignitionDelay  : in  STD_LOGIC_VECTOR(9 downto 0);
    switchedOn, zeroPass    : in  STD_LOGIC;
	 Clock,Reset, CountersClock  : in  STD_LOGIC;
	 triacTriggerPulse : out STD_LOGIC;
	 testData : out std_LOGIC_VECTOR (10  downto 0)
  );
end;

architecture driverJob of triacDriver is
	type triacStateType is (idle,  triacTriggerDelay, fireIgnitionTrigger, fireBreak);
	SIGNAL triacState : triacStateType;
	SIGNAL IgnitionDelayReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL IgnitionDelayCounterReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL aclrIgnitionDelaySig, equalIgnitionDelaySig, equalIgnitionDelaySigA1, equalIgnitionDelaySigA2 : std_LOGIC;

component rustLightCounter
	PORT
	(
		aclr		: IN STD_LOGIC ;
		clock		: IN STD_LOGIC ;
		q		: OUT STD_LOGIC_VECTOR (9 DOWNTO 0)
	);
end component;
component rustLightCompare_1
	PORT
	(
		dataa		: IN STD_LOGIC_VECTOR (9 DOWNTO 0);
		datab		: IN STD_LOGIC_VECTOR (9 DOWNTO 0);
		ageb		: OUT STD_LOGIC 
	);
end component;

  begin
		PROCESS ( Reset, Clock )	
			procedure entryIdle is
			begin
				triacTriggerPulse <= '0';
				triacState <= idle;
				aclrIgnitionDelaySig <= '1';
--				cntEnaIgnitionDelaySig  <= '0';
--				sclrIgnitionDurationSig <= '0';
--				cntEnaIgnitionDurationSig <= '0';
--				sclrFireBreakDurationSig  <= '0';
--				cntEnaFireBreakDurationSig  <= '0';
			end procedure entryIdle;
		BEGIN
			IF ( Reset = '0' ) THEN
				entryIdle;
			ELSIF ( Clock'EVENT AND Clock = '1' ) THEN
				equalIgnitionDelaySigA2 <= equalIgnitionDelaySigA1;
				equalIgnitionDelaySigA1 <= equalIgnitionDelaySig;
				IF ((switchedOn = '0') OR (zeroPass = '0')) THEN 
					entryIdle; 
				ELSE
					IF ((triacState = idle) AND (zeroPass = '1')) THEN
					--  set ignition delay to input in this transition
						IgnitionDelayReg <= ignitionDelay; 
						aclrIgnitionDelaySig <= '0';
						triacState <= triacTriggerDelay;
					END IF;
					IF ((triacState = triacTriggerDelay) AND (equalIgnitionDelaySigA2 = '1'))  THEN
--							OR ((triacState = fireBreak ) AND (equalFireBreakDurationSig = '1')) )   THEN
						aclrIgnitionDelaySig <= '1';
--						triacTriggerPulse <= '1';
					END IF;
--					IF ((triacState = fireIgnitionTrigger)  AND (equalIgnitionDurationSig = '1' ))  THEN
--						triacState <= fireBreak;
--					END IF;
				END IF;   -- not idle	
			END IF;   -- not reset
		END PROCESS ;
		rustLightIgnitionDelayCounter : rustLightCounter PORT MAP (
			clock	 => countersClock,
			aclr	 => aclrIgnitionDelaySig,
			q	 => IgnitionDelayCounterReg
		);
		rustLightIgnitionDelayCompare : rustLightCompare_1 PORT MAP (
			dataa	 => IgnitionDelayCounterReg,
			datab  => IgnitionDelayReg,
			ageb   => equalIgnitionDelaySig
		);
end;
