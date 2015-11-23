project( SQ1 )
if( MSVC )
	cmake_minimum_required(VERSION 2.8.0)
else( MSVC )
	cmake_minimum_required(VERSION 2.6.3)
endif( MSVC )

# ----------------------------------------------------------------------------
#  Print system information:
# ----------------------------------------------------------------------------
message( STATUS "-----------------------------------------------------------------" )
message( STATUS "system information " )
message( STATUS "-----------------------------------------------------------------" )
message( STATUS " system name: ${CMAKE_SYSTEM_NAME}" )
message( STATUS " system processor: ${CMAKE_SYSTEM_PROCESSOR}" )
message( STATUS " system version: ${CMAKE_SYSTEM_VERSION}" )
message( STATUS " msvc version: ${MSVC_VERSION}" )

if( MSVC )
    set( OS_TARGET "win32" )
    set( WINDOWS_TARGET BOOL ON )
        set( PROCESS_FILE_EXT "exe" )
    set( PREFIX "" )
    if( MSVC60 )
        set( COMPILER_VERSION "vs6" )
    elseif( MSVC71 )
        set( COMPILER_VERSION "vs2003" )
    elseif( MSVC80 )
        set( COMPILER_VERSION "vs2005" )
    elseif( MSVC90 )
        set( COMPILER_VERSION "vs2008" )
    elseif( MSVC10 )
		set( COMPILER_VERSION "vs2010" )
	elseif( MSVC11 )
		set( COMPILER_VERSION "vs2012" )
	elseif( MSVC12 )
		set( COMPILER_VERSION "vs2013" )
	endif()
	#if( COMPILER_VERSION )
	#	set( OS_TARGET ${COMPILER_VERSION} )
    #endif() 
elseif( CMAKE_SYSTEM_PROCESSOR STREQUAL "arm" ) #armv6l
    set( PREFIX "lib" )
    set( OS_TARGET "armv6l")
    set( ARM_TARGET BOOL ON)
    message( STATUS "target board is armv6l")
elseif( CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l" ) 
    set( PREFIX "lib" )
    set( OS_TARGET "armv7l")
    set( ARMV7L_TARGET BOOL ON)
    message( STATUS "target board is armv7l")
elseif( CMAKE_SYSTEM_PROCESSOR STREQUAL "i686" )
    set( OS_TARGET "gnulinux")
    set( LINUX_TARGET BOOL ON )
    set( UBUNTU_TARGET BOOL ON )
    set( COMPONENT_EXT "so" )
    set( PREFIX "lib" )
    #if( CMAKE_SYSTEM_VERSION STREQUAL "2.6.32-33-generic" )
    #       set( UBUNTU_10_04 BOOL ON )
    #elseif ( CMAKE_SYSTEM_VERSION STREQUAL "2.6.31-22-generic" )
    #       set( UBUNTU_09_03 BOOL ON )
    #endif()
    message( STATUS "target board is i686")
else()
    message( STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}" )
    message( FATAL_ERROR "Unknown target!!!" )
endif()
message( STATUS " compiler verson : ${COMPILER_VERSION}" )
message( STATUS " OS target : ${OS_TARGET}" )
# Detect machine architecture, on UNIX:
if( UNIX )
	execute_process( COMMAND uname -m
		OUTPUT_VARIABLE CMAKE_RLAB_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE )
	message( STATUS "Architecture uname -m: ${CMAKE_RLAB_ARCH}" )
endif( UNIX )
message( STATUS "-----------------------------------------------------------------" )

# ----------------------------------------------------------------------------
# Indicate CMake 2.7 and above that we don't want to mix relative
#  and absolute paths in linker lib lists.
# Run "cmake --help-policy CMP0003" for more information.
# ----------------------------------------------------------------------------
if( COMMAND cmake_policy )
  cmake_policy( SET CMP0003 NEW )
endif()

# ----------------------------------------------------------------------------
# In case of Makefiles if the user does not setup CMAKE_BUILD_TYPE, assume it's Release:
# ----------------------------------------------------------------------------
if( ${CMAKE_GENERATOR} MATCHES ".*Makefiles" )
    if( "${CMAKE_BUILD_TYPE}" STREQUAL "" )
        set( CMAKE_BUILD_TYPE Release )
    endif()
endif()

# ----------------------------------------------------------------------------
# Enable precompiled headers:
# ----------------------------------------------------------------------------
if( MSVC )
	set(DEFAULT_VAL_PRECOMP_HDR OFF )
else( MSVC)
	set( DEFAULT_VAL_PRECOMP_HDR OFF )
endif( MSVC )
set( SQ1_ENABLE_PRECOMPILED_HDRS ${DEFAULT_VAL_PRECOMP_HDR} CACHE BOOL "Enable precompiled headers" )
mark_as_advanced( SQ1_ENABLE_PRECOMPILED_HDRS )

# ---------------------------------------------------------------------------
# The SQ1 include & source directories:
# ---------------------------------------------------------------------------
set( SQ1_APPS_DIR "${CMAKE_SOURCE_DIR}/apps" CACHE INTERNAL "" )
set( SQ1_CAN_DIR "${CMAKE_SOURCE_DIR}/apps" CACHE INTERNAL "" )
set( SQ1_TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools" CACHE INTERNAL "" )

# ---------------------------------------------------------------------------
# The C++ include & link directories:
# ---------------------------------------------------------------------------
# general include directory
include_directories( include )
include_directories( lib )

# ----------------------------------------------------------------------------
# Select CAN interface:
# ----------------------------------------------------------------------------
set( CAN_USE_PEAK ON CACHE BOOL "" )
set( CAN_USE_KVASER OFF CACHE BOOL "" )
set( CAN_USE_IXXAT OFF CACHE BOOL "" )
set( CAN_USE_ESD OFF CACHE BOOL "" )
set( CAN_USE_NI OFF CACHE BOOL "" )

set( PEAK_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/Peak" CACHE PATH "" )
set( PEAK_LIBRARY "${CMAKE_SOURCE_DIR}/lib/x86/Peak/PCANBasic.lib" CACHE PATH "" )
set( PEAK_LIBRARY_DEBUG "${CMAKE_SOURCE_DIR}/lib/x86/Peak/PCANBasic.lib" CACHE PATH "" )
set( KVASER_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/Kvaser" CACHE PATH "" )
set( KVASER_LIBRARY "${CMAKE_SOURCE_DIR}/lib/x86/Kvaser/canlib32.lib" CACHE PATH "" )
set( KVASER_LIBRARY_DEBUG "${CMAKE_SOURCE_DIR}/lib/x86/Kvaser/canlib32.lib" CACHE PATH "" )
set( IXXAT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/IXXAT" CACHE PATH "" )
set( IXXAT_LIBRARY "${CMAKE_SOURCE_DIR}/lib/x86/IXXAT/vcisdk.lib" CACHE PATH "" )
set( IXXAT_LIBRARY_DEBUG "${CMAKE_SOURCE_DIR}/lib/x86/IXXAT/vcisdk.lib" CACHE PATH "" )
set( ESD_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/ESD-CAN" CACHE PATH "" )
set( ESD_LIBRARY "${CMAKE_SOURCE_DIR}/lib/x86/ESD-CAN/ntcan.lib" CACHE PATH "" )
set( ESD_LIBRARY_DEBUG "${CMAKE_SOURCE_DIR}/lib/x86/ESD-CAN/ntcan.lib" CACHE PATH "" )
set( NI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/NI-CAN" CACHE PATH "" )
set( NI_LIBRARY "${CMAKE_SOURCE_DIR}/lib/x86/NI-CAN/nicanmsc.lib" CACHE PATH "" )
set( NI_LIBRARY_DEBUG "${CMAKE_SOURCE_DIR}/lib/x86/NI-CAN/nicanmsc.lib" CACHE PATH "" )

# ----------------------------------------------------------------------------
# Set CAN log level(detail):
# ----------------------------------------------------------------------------
set( CAN_PRINT_Rx_MESSAGE OFF CACHE BOOL "" )
set( CAN_PRINT_Tx_MESSAGE OFF CACHE BOOL "" )
set( CAN_PRINT_SDO_RESPONSE ON CACHE BOOL "" )
set( CAN_PRINT_DIAGNOSIS ON CACHE BOOL "" )

# ----------------------------------------------------------------------------
# Build options:
# ----------------------------------------------------------------------------
set( BUILD_APPS ON CACHE BOOL "" )
set( BUILD_CAN_SAMPLES OFF CACHE BOOL "" )
set( BUILD_TOOLS OFF CACHE BOOL "" )

# ---------------------------------------------------------------------------
# Build libraries and plug-ins:
# ---------------------------------------------------------------------------
add_subdirectory( apps )
add_subdirectory( tools )
