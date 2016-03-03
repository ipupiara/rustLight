LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;

-- Top-level entity
ENTITY rustLight IS
	GENERIC ( amtTriacs : INTEGER := 4 ) ;
	PORT ( 	data : IN STD_LOGIC_VECTOR(9 DOWNTO 0) ;
				address : IN STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
				Clock, Reset, Strobe, zeroPass, SwitchOnOff : IN STD_LOGIC;
				triacTriggerPulses : OUT STD_LOGIC_VECTOR(amtTriacs - 1 DOWNTO 0) 
			 ) ;
END rustLight ;
ARCHITECTURE Behavior OF rustLight IS
	SIGNAL DataReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL SwitchOnOffReg, StrobeReg, zeroPassReg: STD_LOGIC;
	SIGNAL AddressReg : STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
	SIGNAL InputReg : STD_LOGIC_VECTOR( (amtTriacs * 11) -1 DOWNTO 0) ;
	SIGNAL TriacInputReg : STD_LOGIC_VECTOR( (amtTriacs * 11) -1 DOWNTO 0) ;	
	
	
	type triacDriverIF is 
	RECORD
		Dta : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
		SwitchNFF : STD_LOGIC;
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
			 switchOnOff, zeroPass    : in  STD_LOGIC;
			 Clock,Reset    : in  STD_LOGIC;
			 triacTriggerPulse : out STD_LOGIC
		  );
	END COMPONENT;
	
	BEGIN
		StrobeReg <= Strobe;
		zeroPassReg <= zeroPass;
			
		demuxer : DeMUX_1toX_N_bits
			GENERIC MAP (PORTS => amtTriacs , BITS => 11)
			PORT MAP (AddressReg,DataReg & SwitchOnOffReg, InputReg);
			
		GEN_REG: 
		for i1 in 0 to amtTriacs-1 generate
			REGX : triacDriver port map
				(InputReg((((i1 + 1) * 11 ) - 2)  downto ((i1 * 11 )  )),
				 InputReg((((i1 + 1) * 11 ) - 1)), zeroPassReg, Clock, Reset,
				 triacTriggerPulses(i1));
		end generate GEN_REG;	
			
		PROCESS ( Reset, Clock )
		BEGIN
			IF Reset = '1' THEN
				DataReg <= (OTHERS => '0'); 
				addressReg  <= (OTHERS => '0'); 
				SwitchOnOffReg <= '0';
			ELSIF ( (Clock'EVENT AND Clock = '1' ) AND (StrobeReg = '1') ) THEN
				DataReg <= data; 
				addressReg <= address;
				SwitchOnOffReg <= switchOnOff; 
				
	--			TriacInputReg <= InputReg;
	--			gen : for i1 in 0 to amtTriacs - 1 loop
	--			  TriacInputReg((((i1 + 1) * 11 ) - 1)  downto ((i1 * 11 ) +1 ))  <= TriacDriverInput(i1).Dta;
	--			  TriacInputReg( (i1 * 11  ))  <= TriacDriverInput(i1).SwitchNFF;
	--			end loop;
				
			END IF ;
		END PROCESS ;

END Behavior ;

