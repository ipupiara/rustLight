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
	SIGNAL sClrIgnitionDelaySig, agebIgnitionDelaySig, agebIgnitionDelaySigA1, agebIgnitionDelaySigA2 : std_LOGIC;
	SIGNAL cnt_En_IgnitionDelay, testSig : std_Logic;
	SIGNAL cnt_En_fireCounter, sclr_fireCounter, ageb_fireCounter : std_LOGIC;
	SIGNAL q_fireCounter, dataa_fireCounter : STD_LOGIC_VECTOR (13 DOWNTO 0);
	SIGNAL cnt_En_fireDelayCounter, sclr_fireDelayCounter, ageb_fireDelayCounter : std_LOGIC;
	SIGNAL q_fireDelayCounter, dataa_fireDelayCounter : STD_LOGIC_VECTOR (13 DOWNTO 0);

SIGNAL  triacDelayClock, sysClock: std_LOGIC;	
SIGNAL  testclocks : STD_LOGIC_VECTOR (1 downto 0);
	
	component rustLightCounter
		PORT
		(
			sclr		: IN STD_LOGIC ;
			clock		: IN STD_LOGIC ;
			cnt_en		: IN STD_LOGIC ;
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

	component rl_sync_counter
		PORT
		(
			clock		: IN STD_LOGIC ;
			cnt_en		: IN STD_LOGIC ;
			sclr		: IN STD_LOGIC ;
			q		: OUT STD_LOGIC_VECTOR (13 DOWNTO 0)
		);
	end component;
	
	component rl_sync_Compare
	PORT
	(
		dataa		: IN STD_LOGIC_VECTOR (13 DOWNTO 0);
		datab		: IN STD_LOGIC_VECTOR (13 DOWNTO 0);
		ageb		: OUT STD_LOGIC 
	);
	end component;
	
	component triacDriverProbe
		PORT
		(
			probe		: IN STD_LOGIC_VECTOR (24 DOWNTO 0);
			source		: OUT STD_LOGIC_VECTOR (1 DOWNTO 0)
		);
	end component;



  begin
--		dataa_fireCounter      <= "00000111111111" ;
--		dataa_fireDelayCounter <= "00111111111111" ;
--		sysClock <= Clock;
--		triacDelayClock <= countersClock ;

--    some test values for jtag debugging 		
		dataa_fireCounter      <= "00000000000111" ;
		dataa_fireDelayCounter <= "00000000001111" ;
		sysClock <= testclocks (0);
		triacDelayClock <= testclocks (1);

	  testData  <= ignitionDelay & switchedOn; 
			
	  rustLightIgnitionDelayCounter : rustLightCounter PORT MAP (
--			clock	 => countersClock,
			clock  => triacDelayClock,
			sclr	 => sClrIgnitionDelaySig,
			cnt_en => cnt_En_IgnitionDelay,
			q	 => IgnitionDelayCounterReg
		);
		rustLightIgnitionDelayCompare : rustLightCompare_1 PORT MAP (
			dataa	 => IgnitionDelayCounterReg,
			datab  => IgnitionDelayReg,
			ageb   => agebIgnitionDelaySig
		);
		rustLightFireCounter : rl_sync_counter PORT MAP (
--			clock => clock,
			clock => sysClock,
			cnt_en => cnt_En_fireCounter,
			sclr =>  sclr_fireCounter ,
			q =>  q_fireCounter
		);
		rustLightFireCompare : rl_sync_Compare PORT MAP (
			dataa => q_fireCounter,
			datab => dataa_fireCounter,
			ageb => ageb_fireCounter
		);
		rustLightFireDelayCounter : rl_sync_counter PORT MAP (
--			clock => clock,
			clock => sysClock,
			cnt_en => cnt_En_fireDelayCounter,
			sclr =>  sclr_fireDelayCounter,
			q =>  q_fireDelayCounter
		);
		rustLightFireDelayCompare : rl_sync_Compare PORT MAP (
			dataa => q_fireDelayCounter,
			datab => dataa_fireDelayCounter,
			ageb => ageb_fireDelayCounter
		);		
		
		triacDriverProbe_inst : triacDriverProbe PORT MAP (
			probe	 => IgnitionDelayCounterReg & IgnitionDelayReg & agebIgnitionDelaySig & ageb_fireCounter & ageb_fireDelayCounter & zeroPass & switchedOn , 
			source	 => testclocks
		);
	
		
		PROCESS ( Clock )	
--		PROCESS ( Clock, Reset )			
			procedure entryIdle is
			begin
				triacTriggerPulse <= '0';
				triacState <= idle;
				sClrIgnitionDelaySig <= '1';
				cnt_En_IgnitionDelay  <= '0';
				sclr_fireCounter <= '1';
				cnt_En_fireCounter <= '0';
				sclr_fireDelayCounter  <= '1';
				cnt_En_fireDelayCounter  <= '0';
			end procedure entryIdle;
		BEGIN
		IF (Reset = '0') THEN
				entryIdle;
			ELSIF ( Clock'EVENT AND Clock = '1' ) THEN
				agebIgnitionDelaySigA2 <= agebIgnitionDelaySigA1;
				agebIgnitionDelaySigA1 <= agebIgnitionDelaySig;
				
				IF ((switchedOn = '0') OR (zeroPass = '0')) THEN 
					entryIdle; 
					IgnitionDelayReg <= ignitionDelay; 
				ELSE
					IF ((triacState = idle) AND (zeroPass = '1')) THEN
						sClrIgnitionDelaySig <= '0';
						cnt_En_IgnitionDelay <= '1';
						triacState <= triacTriggerDelay;
					END IF;
					IF (((triacState = triacTriggerDelay) AND (agebIgnitionDelaySigA2 = '1'))  
							OR ((triacState = fireBreak ) AND (ageb_fireDelayCounter = '1')) )   THEN	
						sClrIgnitionDelaySig <= '1';
						cnt_En_IgnitionDelay <= '0';
						
						sclr_fireDelayCounter <= '1';
						cnt_En_fireDelayCounter <='0';
						
						sclr_fireCounter <= '0';
						cnt_En_fireCounter <= '1';
						
						triacTriggerPulse <= '1';
						triacState <= fireIgnitionTrigger;
					END IF;
					IF ((triacState = fireIgnitionTrigger)  AND (ageb_fireCounter = '1' ))  THEN
						triacTriggerPulse <= '0';
						
						sclr_fireCounter <= '1';						
						cnt_En_fireCounter <= '0';
						
						sclr_fireDelayCounter <= '0';
						cnt_En_fireDelayCounter <='1';
						triacState <= fireBreak;
					END IF;
				END IF;  -- not zero or not switchedOn
			END IF;   -- not reset and clock 1
		END PROCESS ;

end;
