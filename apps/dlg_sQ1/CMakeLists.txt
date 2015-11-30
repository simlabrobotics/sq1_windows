set( name dlg_sQ1 )


# import 3rdparty:
set( SQ1_DEP_LINK_LIBS  )

macro( config_project PROJNAME LIBNAME )
	include_directories( ${${LIBNAME}_INCLUDE_DIR} )
	set( SQ1_DEP_LINK_LIBS ${SQ1_DEP_LINK_LIBS} optimized "${${LIBNAME}_LIBRARY}" )
	set( SQ1_DEP_LINK_LIBS ${SQ1_DEP_LINK_LIBS} debug "${${LIBNAME}_LIBRARY_DEBUG}" )
endmacro()

if( wxWidgets_FOUND )
	include_directories( ${wxWidgets_INCLUDE_DIRS} )
	set( SQ1_DEP_LINK_LIBS ${RLAB_DEP_LINK_LIBS} "${wxWidgets_LIBRARIES}" )
endif()

# include CAN header & source files:
set( ${name}_EXTRA_SRCS "" )
set( ${name}_EXTRA_SRCS_NAME "" )

LIST(APPEND ${name}_EXTRA_SRCS 
	"${CMAKE_SOURCE_DIR}/include/sq1_*.h"
	)
LIST(APPEND ${name}_EXTRA_SRCS_NAME 
	"common header"
	)

macro(add_CAN_srcs CAN_name)
	LIST(APPEND ${name}_EXTRA_SRCS 
		"${SQ1_CAN_DIR}/${CAN_name}/*.cpp"
		"${SQ1_CAN_DIR}/${CAN_name}/*.c"
		"${SQ1_CAN_DIR}/${CAN_name}/*.cxx"
		"${SQ1_CAN_DIR}/${CAN_name}/*.h"
		"${SQ1_CAN_DIR}/${CAN_name}/*.hpp"
		"${SQ1_CAN_DIR}/${CAN_name}/*.rc"
		"${CMAKE_SOURCE_DIR}/include/can*.h"
		"${CMAKE_SOURCE_DIR}/include/${CAN_name}/*.cpp"
		"${CMAKE_SOURCE_DIR}/include/${CAN_name}/*.c"
		"${CMAKE_SOURCE_DIR}/include/${CAN_name}/*.cxx"
		"${CMAKE_SOURCE_DIR}/include/${CAN_name}/*.h"
		"${CMAKE_SOURCE_DIR}/include/${CAN_name}/*.hpp"
		"${CMAKE_SOURCE_DIR}/include/${CAN_name}/*.rc"
		"${SQ1_APPS_DIR}/can*.cpp"
		"${SQ1_APPS_DIR}/can*.c"
		)

	LIST(APPEND ${name}_EXTRA_SRCS_NAME
		"${CAN_name} CAN source"
		"${CAN_name} CAN source"
		"${CAN_name} CAN source"
		"${CAN_name} CAN header"
		"${CAN_name} CAN header"
		"${CAN_name} CAN resource"
		"${CAN_name} CAN header"
		"${CAN_name} CAN source"
		"${CAN_name} CAN source"
		"${CAN_name} CAN source"
		"${CAN_name} CAN header"
		"${CAN_name} CAN header"
		"${CAN_name} CAN resource"
		"${CAN_name} CAN source"
		"${CAN_name} CAN source"
		)
		
	add_definitions( -D${CAN_name}CAN	)
endmacro(add_CAN_srcs CAN_name)

if( CAN_USE_PEAK )
	add_CAN_srcs( Peak )
	config_project( ${name} PEAK )
elseif ( CAN_USE_KVASER )
	add_CAN_srcs( Kvaser )
	config_project( ${name} Kvaser )
elseif ( CAN_USE_IXXAT )
	add_CAN_srcs( IXXAT )
	config_project( ${name} IXXAT )
elseif ( CAN_USE_ESD )
	add_CAN_srcs( ESD-CAN )
	config_project( ${name} ESD )
elseif ( CAN_USE_NI )
	add_CAN_srcs( NI-CAN )
	config_project( ${name} NI )
endif()

LIST(LENGTH ${name}_EXTRA_SRCS N_SRCS)
LIST(LENGTH ${name}_EXTRA_SRCS_NAME N_SRCS_NAMES)

IF (NOT N_SRCS EQUAL N_SRCS_NAMES)
	MESSAGE(FATAL_ERROR "Mismatch length in ${name}_EXTRA_SRCS and ${name}_EXTRA_SRCS_NAME!")
ENDIF (NOT N_SRCS EQUAL N_SRCS_NAMES)

SET(${name}_srcs "")  # ALL the files

MATH(EXPR N_SRCS "${N_SRCS}-1")  # Indices are 0-based

foreach(i RANGE 0 ${N_SRCS})
	# Get i'th expression & its name:
	LIST(GET ${name}_EXTRA_SRCS      ${i} FILS_EXPR)
	LIST(GET ${name}_EXTRA_SRCS_NAME ${i} FILS_GROUP_NAME)
	
	FILE(GLOB aux_list ${FILS_EXPR})
	
	SOURCE_GROUP("${FILS_GROUP_NAME} files" FILES ${aux_list})
	
	# Add to main list:
	LIST(APPEND ${name}_srcs ${aux_list})
	# All to group lists, may be used by the user upon return from this macro:
	LIST(APPEND ${FILS_GROUP_NAME}_FILES ${aux_list})
endforeach(i)

set(all_${name}_srcs 
	${${name}_srcs}
	"sQ1App.cpp"
	"sQ1App.h"
	"sQ1CANImpl.cpp"
	"sQ1CANImpl.h"
	)

	
# Add special defines needed by this device, if any:
add_definitions(
	-D_UNICODE	
	-D__WXMSW__
	-DwxUSE_CMDLINE_PARSER
	)	
	
# Build demo:
message( STATUS " Add application: ${name}" )
add_executable( ${name} WIN32 ${all_${name}_srcs} )
target_link_libraries( ${name}  ${SQ1_DEP_LINK_LIBS} )
