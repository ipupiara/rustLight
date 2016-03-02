LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;

-- demuxer

entity triacDriver is
  port (
    ignitionDelay  : in  STD_LOGIC_VECTOR(9 downto 0);
    switchOnOff, zeroPass    : in  STD_LOGIC;
	 Clock,Reset    : in  STD_LOGIC
  );
end;

architecture driverJob of triacDriver is
	SIGNAL IgnitionDelayReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL SwitchOnOffReg, ZeroPassReg   : STD_LOGIC;
  begin
		PROCESS ( Reset, Clock )
		BEGIN
			IF Reset = '1' THEN
				IgnitionDelayReg <= (OTHERS => '0'); 
				SwitchOnOffReg <= '0';
				ZeroPassReg <= '0';
			ELSIF  (Clock'EVENT AND Clock = '1' ) THEN
				IgnitionDelayReg <= ignitionDelay; 
				SwitchOnOffReg <= switchOnOff; 
				ZeroPassReg <= zeroPass;
			END IF ;
		END PROCESS ;

end;
