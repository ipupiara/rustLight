## quartus_stp
## use quartus_stp to run this file 
puts "tickel tickel script test0.tcl" 

help

project_open rustlight

set usb [lindex [get_hardware_names] 0]
set device_name [lindex [get_device_names -hardware_name $usb] 0]
 
## start_insystem_source_probe -device_name $device_name -hardware_name $usb
## get_insystem_source_probe_instance_info  -device_name $device_name -hardware_name $usb


foreach instance [get_insystem_source_probe_instance_info -hardware_name  $usb -device_name $device_name ] {
	puts "[lindex $instance 0],[lindex $instance 1],[lindex $instance 2],[lindex $instance 3]"
}


end_insystem_source_probe

project_close 

puts " leave the tickel tock tock"