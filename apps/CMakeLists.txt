project( SQ1_EXAMPLES )

if( BUILD_APPS ) 
	add_subdirectory( canopen_TxPDO )
	add_subdirectory( canopen_SDI )
	add_subdirectory( canopen_enum_devices )
endif()

