LIBRARY ieee ;
USE ieee.std_logic_1164.all ;
USE IEEE.MATH_REAL.ALL;
USE IEEE.NUMERIC_STD.ALL;

-- Top-level entity
ENTITY rustLight IS
	GENERIC ( amtTriacs : INTEGER := 4 ) ;
	PORT ( 	data : IN STD_LOGIC_VECTOR(9 DOWNTO 0) ;
				address : IN STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
				Clock, Reset, Strobe, SwitchOnOff : IN STD_LOGIC 
			 ) ;
END rustLight ;
ARCHITECTURE Behavior OF rustLight IS
	SIGNAL DataReg : STD_LOGIC_VECTOR(9 DOWNTO 0) ;
	SIGNAL SwitchOnOffReg: STD_LOGIC;
	SIGNAL AddressReg : STD_LOGIC_VECTOR(integer(ceil(log2(real(amtTriacs)))) - 1 DOWNTO 0) ;
	SIGNAL InputReg : STD_LOGIC_VECTOR( (amtTriacs * 11) -1 DOWNTO 0) ;
	
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
	
	BEGIN
	demuxer : DeMUX_1toX_N_bits
		GENERIC MAP (PORTS => amtTriacs , BITS => 11)
		PORT MAP (AddressReg,DataReg & SwitchOnOffReg, InputReg);
		
	PROCESS ( Reset, Clock )
	BEGIN
		IF Reset = '1' THEN
			DataReg <= (OTHERS => '0'); 
			SwitchOnOffReg <= '0';
			AddressReg <= (OTHERS => '0'); 
		ELSIF ( (Clock'EVENT AND Clock = '1' ) AND (Strobe = '1') ) THEN
			DataReg <= data; 
			SwitchOnOffReg <= switchOnOff; 
			AddressReg <= address;
		END IF ;
	END PROCESS ;


END Behavior ;

