project( SQ1_EXAMPLES )

if( CAN_PRINT_Rx_MESSAGE )
	add_definitions( -DCAN_PRINT_Rx_MESSAGE )
endif()

if( CAN_PRINT_Tx_MESSAGE )
	add_definitions( -DCAN_PRINT_Tx_MESSAGE )
endif()

if( CAN_PRINT_SDO_RESPONSE )
	add_definitions( -DCAN_PRINT_SDO_RESPONSE )
endif()

if( CAN_PRINT_DIAGNOSIS )
	add_definitions( -DCAN_PRINT_DIAGNOSIS )
endif()

# Add demo applications:
if( BUILD_CAN_SAMPLES )
	add_subdirectory( canopen_PDO )
	add_subdirectory( canopen_SDO )
	add_subdirectory( canopen_enum_devices )
	add_subdirectory( canopen_homing )
	add_subdirectory( canopen_binary_interpreter )
endif()

if( BUILD_APPS ) 
	add_subdirectory( app_sQ1 )
endif()

find_package( wxWidgets COMPONENTS base gl core adv aui )
if( wxWidgets_FOUND AND BUILD_APPS )
	add_subdirectory( dlg_sQ1 )
endif()
