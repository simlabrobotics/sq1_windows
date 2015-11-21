project( SQ1_EXAMPLES )

if( CAN_PRINT_Rx_MESSAGE )
	add_definitions( -DCAN_PRINT_Rx_MESSAGE )
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
endif()

if( BUILD_APPS ) 
	add_subdirectory( app_sQ1 )
endif()

