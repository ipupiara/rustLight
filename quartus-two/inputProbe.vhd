-- megafunction wizard: %In-System Sources and Probes%
-- GENERATION: STANDARD
-- VERSION: WM1.0
-- MODULE: altsource_probe 

-- ============================================================
-- File Name: inputProbe.vhd
-- Megafunction Name(s):
-- 			altsource_probe
--
-- Simulation Library Files(s):
-- 			altera_mf
-- ============================================================
-- ************************************************************
-- THIS IS A WIZARD-GENERATED FILE. DO NOT EDIT THIS FILE!
--
-- 13.1.0 Build 162 10/23/2013 SJ Web Edition
-- ************************************************************


--Copyright (C) 1991-2013 Altera Corporation
--Your use of Altera Corporation's design tools, logic functions 
--and other software and tools, and its AMPP partner logic 
--functions, and any output files from any of the foregoing 
--(including device programming or simulation files), and any 
--associated documentation or information are expressly subject 
--to the terms and conditions of the Altera Program License 
--Subscription Agreement, Altera MegaCore Function License 
--Agreement, or other applicable license agreement, including, 
--without limitation, that your use is for the sole purpose of 
--programming logic devices manufactured by Altera and sold by 
--Altera or its authorized distributors.  Please refer to the 
--applicable agreement for further details.


LIBRARY ieee;
USE ieee.std_logic_1164.all;

LIBRARY altera_mf;
USE altera_mf.all;

ENTITY inputProbe IS
	PORT
	(
		probe		: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
		source		: OUT STD_LOGIC_VECTOR (0 DOWNTO 0)
	);
END inputProbe;


ARCHITECTURE SYN OF inputprobe IS

	SIGNAL sub_wire0	: STD_LOGIC_VECTOR (0 DOWNTO 0);



	COMPONENT altsource_probe
	GENERIC (
		enable_metastability		: STRING;
		instance_id		: STRING;
		probe_width		: NATURAL;
		sld_auto_instance_index		: STRING;
		sld_instance_index		: NATURAL;
		source_initial_value		: STRING;
		source_width		: NATURAL;
		lpm_type		: STRING
	);
	PORT (
			probe	: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
			source	: OUT STD_LOGIC_VECTOR (0 DOWNTO 0)
	);
	END COMPONENT;

BEGIN
	source    <= sub_wire0(0 DOWNTO 0);

	altsource_probe_component : altsource_probe
	GENERIC MAP (
		enable_metastability => "NO",
		instance_id => "inPr",
		probe_width => 16,
		sld_auto_instance_index => "YES",
		sld_instance_index => 0,
		source_initial_value => " 0",
		source_width => 1,
		lpm_type => "altsource_probe"
	)
	PORT MAP (
		probe => probe,
		source => sub_wire0
	);



END SYN;

-- ============================================================
-- CNX file retrieval info
-- ============================================================
-- Retrieval info: PRIVATE: INTENDED_DEVICE_FAMILY STRING "Cyclone III"
-- Retrieval info: LIBRARY: altera_mf altera_mf.altera_mf_components.all
-- Retrieval info: CONSTANT: ENABLE_METASTABILITY STRING "NO"
-- Retrieval info: CONSTANT: INSTANCE_ID STRING "inPr"
-- Retrieval info: CONSTANT: PROBE_WIDTH NUMERIC "16"
-- Retrieval info: CONSTANT: SLD_AUTO_INSTANCE_INDEX STRING "YES"
-- Retrieval info: CONSTANT: SLD_INSTANCE_INDEX NUMERIC "0"
-- Retrieval info: CONSTANT: SOURCE_INITIAL_VALUE STRING " 0"
-- Retrieval info: CONSTANT: SOURCE_WIDTH NUMERIC "1"
-- Retrieval info: USED_PORT: probe 0 0 16 0 INPUT NODEFVAL "probe[15..0]"
-- Retrieval info: USED_PORT: source 0 0 1 0 OUTPUT NODEFVAL "source[0..0]"
-- Retrieval info: CONNECT: @probe 0 0 16 0 probe 0 0 16 0
-- Retrieval info: CONNECT: source 0 0 1 0 @source 0 0 1 0
-- Retrieval info: GEN_FILE: TYPE_NORMAL inputProbe.vhd TRUE
-- Retrieval info: GEN_FILE: TYPE_NORMAL inputProbe.inc FALSE
-- Retrieval info: GEN_FILE: TYPE_NORMAL inputProbe.cmp TRUE
-- Retrieval info: GEN_FILE: TYPE_NORMAL inputProbe.bsf FALSE
-- Retrieval info: GEN_FILE: TYPE_NORMAL inputProbe_inst.vhd TRUE
-- Retrieval info: LIB_FILE: altera_mf
