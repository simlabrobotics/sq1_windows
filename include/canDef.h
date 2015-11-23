/*
 *\brief Constants and enums for CAN communication
 *
 *$Author: Sangyup Yi $
 *$Date: 2015/11/18 19:00:00 $
 *$Revision: 1.0 $
 */ 

#ifndef _CANDEF_H
#define _CANDEF_H


#ifdef USING_NAMESPACE_CANAPI
#   define CANAPI_BEGIN namespace CANAPI {
#   define CANAPI_END };
#else
#   define CANAPI_BEGIN
#   define CANAPI_END
#endif



CANAPI_BEGIN


typedef struct{
	unsigned char	STD_EXT;
	unsigned long	msg_id;         // message identifier
   	unsigned char	data_length;    //
   	char			data[8];        // data array
} can_msg;

#define		STD		(bool)0
#define		EXT		(bool)1


#pragma pack(push)
#pragma pack(1)
typedef struct {
	union {
		unsigned char d[8];
		struct {
			union {
				// initiate SDO download request / upload response:
				struct {
					unsigned char s   : 1; // size indicator
					unsigned char e   : 1; // transfer type;
					unsigned char n1  : 2; // number of bytes in data that do not contain data
					unsigned char x1  : 1; // (not used)
					unsigned char cs  : 3; // server command specifier (server to client)
				};
				// initiate SDO download response / upload request:
				struct {
					unsigned char x2  : 5; // (not used)
					unsigned char cs  : 3; // client command specifier (client to server)
				};
				// segmented SDO download request / upload response:
				struct {
					unsigned char c   : 1; // more segments to be uploaded(0: more, 1:no more)
					unsigned char n3  : 3; // number of bytes in data that do not contain data
					unsigned char t   : 1; // toggle bit
					unsigned char cs  : 3; // server command specifier (server to client)
				};
				// segmented SDO download response / upload request:
				struct {
					unsigned char x4  : 4; // (not used)
					unsigned char t   : 1; // toggle bit
					unsigned char cs  : 3; // client command specifier (client to server)
				};
				
				
			};
			unsigned short obj_index;
			unsigned char sub_index;
			unsigned char reserved[4];
		};
	};
//		unsigned char& operator[] (const size_t index) { return d[index]; }
//		const unsigned char& operator[] (const size_t index) const { return d[index]; }
	operator unsigned char*() { return d; }
} SDO_data_t;
#pragma pack(pop)

///////////////////////////////////////////////
//  SDO Client/Server Command Specifier:
#define SDO_CCS_DOWNLOAD				0
#define SDO_CCS_INITIATE_DOWNLOAD		1
#define SDO_CCS_UPLOAD					3
#define SDO_CCS_INITIATE_UPLOAD			2

#define SDO_SCS_DOWNLOAD				1
#define SDO_SCS_INITIATE_DOWNLOAD		3
#define SDO_SCS_UPLOAD					0
#define SDO_SCS_INITIATE_UPLOAD			2

#define SDO_CS_ABORT					4

///////////////////////////////////////////////
//  Define CAN master & slaves ID:
#define NODEID_GLOBAL		0x00
#define NODEID_MASTER		0x00

////////////////////////////////////////////////
//  Define CAN Function code(COB type):
//
//COB Type                      Bits 8 - 11 of COB-ID    ID Range
//----------------------------------------------------------------------------
//NMT                           0000                     0
//SYNC                          0001                     128 (80h)
//Time Stamp                    0010                     256 (100h)
//Emergency                     0001                     129・255 (81h・ffh)
//PDO1 - Transmit               0011                     385・511 (181h・1ffh)
//PDO1 - Receive                0100                     513・639 (201h・27fh)
//PDO2 - Transmit               0101                     641・767 (281h・2ffh)
//PDO2 - Receive                0110                     769・895 (301h・37fh)
//PDO3 - Transmit               0111                     897・1023 (381h・3ffh)
//PDO3 - Receive                1000                     1025・1151 (401h・47fh)
//PDO4 - Transmit               1001                     1153・1279 (481h・4ffh)
//PDO4 - Receive                1010                     1281・1407 (501h・57fh)
//SDO - Transmit                1011                     1409・1535 (581h・5ffh)
//SDO - Receive                 1100                     1537・1663 (601h・67fh)
//Error control (node guarding) 1110                     1793・1919 (701h・77fh)
//
#define COBTYPE_NMT				0X00
#define COBTYPE_SYNC			0x01
#define COBTYPE_TIMESTAMP		0x02
#define COBTYPE_EMERGENCY		0x01
#define COBTYPE_TxPDO1			0x03
#define COBTYPE_RxPDO1			0x04
#define COBTYPE_TxPDO2			0x05
#define COBTYPE_RxPDO2			0x06
#define COBTYPE_TxPDO3			0x07
#define COBTYPE_RxPDO3			0x08
#define COBTYPE_TxPDO4			0x09
#define COBTYPE_RxPDO4			0x0a
#define COBTYPE_TxSDO			0x0b
#define COBTYPE_RxSDO			0x0c
#define COBTYPE_ERRCTL			0x0e

static const char* COBTYPE_NAME(unsigned char fn_code)
{
	switch (fn_code)
	{
	case COBTYPE_NMT: return "NMT";
	case COBTYPE_SYNC: return "SYNC_EMERGENCY";
	case COBTYPE_TIMESTAMP: return "TIMESTAMP";
	//case COBTYPE_EMERGENCY: return "EMERGENCY";
	case COBTYPE_TxPDO1: return "TxPDO1";
	case COBTYPE_RxPDO1: return "RxPDO1";
	case COBTYPE_TxPDO2: return "TxPDO2";
	case COBTYPE_RxPDO2: return "RxPDO2";
	case COBTYPE_TxPDO3: return "TxPDO3";
	case COBTYPE_RxPDO3: return "RxPDO3";
	case COBTYPE_TxPDO4: return "TxPDO4";
	case COBTYPE_RxPDO4: return "RxPDO4";
	case COBTYPE_TxSDO: return "TxSDO";
	case COBTYPE_RxSDO: return "RxSDO";
	case COBTYPE_ERRCTL: return "ERRCTL";
	}
	return "Unknown";
}

///////////////////////////////////////////////
//  Define special COB-ID:
#define COBID_NMT				0x0000
#define COBID_SYNC				0x0080
#define COBID_TIMESTAMP			0X0100
#define COBID_LSS_REQ			0x07e5
#define COBID_LSS_ACK			0x07e5

////////////////////////////////////////////////
//  Define CAN Object Dictionary Index:
//	0 Not used
//	0001 - 001F Static data type
//	0020 - 003F Complex data type
//	0040 - 005F Manufacturer-specific data type
//	0060 - 0FFF Reserved
//	1000 - 1FFF Communication profile area
//	2000 - 2FFF Manufacturer-specific profile area
//	6000 - 6FFF Standardized device profile area
//	A000 - FFF Reserved
#define OD_DEVICE_TYPE							0x1000
#define OD_ERROR_REGISTER						0x1001
#define OD_STATUS_REGISTER						0x1002
#define OD_PREDEFINED_ERROR_FIELD				0x1003
#define OD_COBID_SYNC							0x1005
#define OD_COMM_CYCLE_PERIOD					0x1006
#define OD_DEVICE_NAME							0x1008
#define OD_HW_VERSION							0x1009
#define OD_SW_VERSION							0x100A
#define OD_NODEID								0x100B
#define OD_STORE_PARAMS							0x1010
#define OD_RESTORE_PARAMS						0x1011
#define OD_COBID_TIMESTAMP						0x1012
#define OD_HR_TIMESTAMP							0x1013
#define OD_COBID_EMERGENCY						0x1014
#define OD_CONSUMER_HB_TIME						0x1016
#define OD_PRODUCER_HB_TIME						0x1017
#define OD_LSS_ADDRESS							0x1018
#define OD_OS_INTERPRETER						0x1023
#define OD_OS_COMMAND_MODE						0x1024
#define OD_ERROR_BEHAVIOR						0x1029
#define OD_SDO1_SERVER							0x1200
#define OD_RxPDO1_COMM_PARAM					0x1400
#define OD_RxPDO2_COMM_PARAM					0x1401
#define OD_RxPDO3_COMM_PARAM					0x1402
#define OD_RxPDO4_COMM_PARAM					0x1403
#define OD_RxPDO1_MAPPING						0x1600
#define OD_RxPDO2_MAPPING						0x1601
#define OD_RxPDO3_MAPPING						0x1602
#define OD_RxPDO4_MAPPING						0x1603
#define OD_TxPDO1_COMM_PARAM					0x1800
#define OD_TxPDO2_COMM_PARAM					0x1801
#define OD_TxPDO3_COMM_PARAM					0x1802
#define OD_TxPDO4_COMM_PARAM					0x1803
#define OD_TxPDO1_MAPPING						0x1A00
#define OD_TxPDO2_MAPPING						0x1A01
#define OD_TxPDO3_MAPPING						0x1A02
#define OD_TxPDO4_MAPPING						0x1A03
// Manufacturer(ELMO)-specific profile area
#define OD_PVT_DATA								0x2001
#define OD_PT_DATA								0x2002
#define OD_FAST_POSITION						0x2003
#define OD_ECAM_DATA							0x2004
#define OD_BIN_INTERPRETER_INPUT				0x2012
#define OD_BIN_INTERPRETER_OUTPUT				0x2013
#define OD_RECORDED_DATA_OUTPUT					0x2030
#define OD_GROUP_ID								0x2040
#define OD_AMP_FREE_RUNNING_TIME				0x2041
#define OD_CAN_CONTROLLER_STATUS				0x2082
#define OD_BEGIN_ON_TIME						0x208A
#define OD_FIRMWARE_DOWNLOAD					0x2090
#define OD_AUX_POSITION_ACTUAL_VALUE			0x20A0
#define OD_POSITION_ERROR						0x20A1
//#define OD_DIGITAL_INPUT						0x2200
#define OD_DIGITAL_INPUTS_LOW_BYTE				0x2201
#define OD_USER_INTEGER							0x2F00
#define OD_USER_FLOAT_ARRAY						0x2F01
#define OD_ET_ARRAY								0x2F02
#define OD_PVT_BUFFER_HEAD_POINTER				0x2F11
#define OD_PVT_BUFFER_TAIL_POINTER				0x2F12
#define OD_BUFFERED_PTP_REMAINED_POINT			0x2F15
#define OD_ASYNC_PDO_EVENT						0x2F20
#define OD_EMERGENCY_EVENT						0x2F21
#define OD_BUSOFF_TIMEOUT						0x2F22
#define OD_DIGITAL_INPUT_TPDO_EVENT_PARAMS		0x2F23
#define OD_LAST_TIMESTAMP_CORRECTION			0x2F30
#define OD_INTERNAL_USEC_COUNTER_AT_LAST_SYNC	0x2F31
#define OD_CONFIGURATION_OBJ					0x2F40
// DSP 402 device specific profile area
#define OD_ABORD_CONNECTION_OPTION_CODE			0x6007 // Function to perform on heartbeat event. (Link) R/W N
#define OD_ERROR_CODE							0x603F // Captures the last error R N
#define OD_CONTROLWORD							0x6040 // Allows changing of drive states. R/W Y
#define OD_STATUSWORD							0x6041 // Indicates current drive status. R Y
#define OD_QUICKSTOP_OPTION_CODE				0x605A // Sets the quick stop option code. R/W N
#define OD_SHUTDOWN_OPTION_CODE					0x605B // Sets the shut down option code. R/W N
#define OD_DISABLE_OPERATION_OPTION_CODE		0x605C // Sets the disable operation option code. R/W N
#define OD_HALT_OPTION_CODE						0x605D // Sets the Halt option code. R/W N
#define OD_FALUT_REACTION_OPTION_CODE			0x605E // Sets drive reaction when fault occurs. R/W N
#define OD_MODE_OF_OPERATION					0x6060 // Sets mode of operation R/W Y
#define OD_MODE_OF_OPERATION_DISPLAY			0x6061 // Displays actual mode of operation. R N
#define OD_POSITION_DEMAND_VALUE				0x6062 // Output of profiler. Position command. R Y
#define OD_ACTUAL_POSITION_INTERNAL_UNIT		0x6063 // Actual position taken from position sensor, in increments. R Y
#define OD_POSITION_ACTUAL_VALUE				0x6064 // Actual position as taken from position sensor, in user units. R Y
#define OD_POSITION_FOLLOWING_ERROR_WINDOW		0x6065 // Defines a range of tolerated position values symmetrical to the position demand value. R/W N
#define OD_POSITION_FOLLOWING_ERROR_WINDOW_TIME	0x6066 // Defines the timeout for the next error window to set the following error indication. R/W N
#define OD_POSITION_WINDOW						0x6067 // Defines a symmetrical position window for the target position for target reached indication. R/W N
#define OD_POSITION_WINDOW_TIME					0x6068 // Defines the time in which the position within the position window indicates target reached. R/W N
#define OD_VELOCITY_SENSOR_ACTUAL_VALUE			0x6069 // Actual velocity as calculated from the main velocity sensor, in increments. R Y
#define OD_VELOCITY_SENSOR_SELECTION_CODE		0x606A // Selects the velocity sensor reading from either the position or the velocity sensor. R/W N
#define OD_VELOCITY_DEMAND_VALUE				0x606B // Demand value for velocity controller. R Y
#define OD_VELOCITY_ACTUAL_SENSOR				0x606C // Actual velocity from either position or velocity sensor. R Y
#define OD_VELOCITY_WINDOW						0x606D // Monitors whether required target velocity was achieved. R/W N
#define OD_VELOCITY_WINDOW_TIME					0x606E // Defines the time in which a target velocity is considered as reached. R/W N
#define OD_VELOCITY_THRESHOLD					0x606F // Defines the value in which the velocity is considered to be 0. R/W N
#define OD_VELOCITY_THRESHOLD_TIME				0x6070 // Defines (with object 0x607F) the time in which the velocity is considered to be 0. R/W N
#define OD_TARGET_TORQUE						0x6071 // The input value for the torque controller in profile torque mode. R/W Y
#define OD_MAX_TORQUE							0x6072 // The maximum permissible torque in the motor. R/W N
#define OD_MAX_CURRENT							0x6073 // The maximum permissible torque creating current in the motor. R/W N
#define OD_TORQUE_DEMAND_VALUE					0x6074 // The maximum permissible torque creating current in the motor. R N
#define OD_MOTOR_RATED_CURRENT					0x6075 // This value is taken from the motor nameplate. R/W N
#define OD_MOTOR_RATED_TORQUE					0x6076 // This value is taken from the motor name plate. R/W N
#define OD_TORQUE_ACTUAL_VALUE					0x6077 // The instantaneous torque in the drive motor. R Y
#define OD_CURRENT_ACTUAL_VALUE					0x6078 // The instantaneous current in the drive motor. R Y
#define OD_PROFILED_TARGET_POSITION				0x607A // Defines target position for absolute or relative point-to-point motion. R/W Y
#define OD_POSITION_RANGE_LIMIT					0x607B // Sets the limits in which the position numerical values are available. R/W N
#define OD_HOMING_OFFSET						0x607C // Defines offset from homing zero position to application zero position. R/W N
#define OD_SOFTWARE_POSITION_LIMIT				0x607D // Defines limits for demand position value and actual position value. R/W N
#define OD_POLARITY								0x607E // Sets polarity for position or speed command and actual value. /W Y
#define OD_MAX_PROFILE_VELOCITY					0x607F // Defines limit to which a profile velocity speed is saturated. R/W N
#define OD_PROFILE_VELOCITY						0x6081 // Sets the speed for the profile position motion. R/W Y
#define OD_PROFILE_ACCELERATION					0x6083 // Defines the acceleration for the profile velocity and profile position motion. R/W Y
#define OD_PROFILE_DECCELERATION				0x6084 // Defines deceleration for profile velocity and profile position motion. R/W N
#define OD_QUICKSTOP_DECCELERATION				0x6085 // Sets the deceleration for a quick stop state. R/W N
#define OD_MOTION_PROFILE_TYPE					0x6086 // Defines method by which profile motion is evaluated (linear or jerk). R/W N
#define OD_TORQUE_SLOPE							0x6087 // the rate of change of torque R/W Y 
#define OD_TORQUE_PROFILE_TYPE					0x6088 // Used to select the type of torque profile used to perform a torque change. R/W N
#define OD_POISTION_NOTATION_INDEX				0x6089 // Used to scale position objects. R/W N
#define OD_POISTION_DIMENSION_INDEX				0x608A // This object defines the position dimension index. R/W N
#define OD_VELOCITY_NOTATION_INDEX				0x608B // This is defined by the physical dimensions and calculated by unit type. R/W N
#define OD_VELOCITY_DIMENSION_INDEX				0x608C // This is used together with the velocity notation index to define a unit. R/W N
#define OD_ACCELERATION_NOTATION_INDEX			0x608D // The unit is defined by the physical dimensions and calculated by unit type and exponent. R/W N
#define OD_ACCELERATION_DIMENSION_INDEX			0x608E // This defines the acceleration dimension index, which is used together with the acceleration notation index (object 0x608D) to define a unit. R/W N
#define OD_POSITION_ENC_RESOLUTION				0x608F // Defines relation between motor revolution and position increments. R/W N
#define OD_Velocity_encoder_resolution			0x6090 // Defines ratio of encoder increments/sec per motor revolutions/sec. R/W N
#define OD_POSITION_FACTOR						0x6093 // Converts position in user units to position in internal increments. R/W N
#define OD_VELOCITY_ENCODER_FACTOR				0x6094 // Converts desired velocity in velocity units into internal increments/sec. R/W N
#define OD_VELOCITY_FACTOR_1					0x6095 // Converts motor data into velocity data. R/W N
#define OD_VELOCITY_FACTOR_2					0x6096 // Converts encoder data for position into encoder data for velocity. R/W N
#define OD_ACCELERATION_FACTOR					0x6097 // Converts the acceleration from user units to internal increments/sec. R/W N
#define OD_HOMING_METHOD						0x6098 // Defines method by which homing procedure is performed. R/W N
#define OD_HOMING_SPEED							0x6099 // Sets speed for homing procedure. R/W N 
#define OD_HOMING_ACCELERATION					0x609A // Sets acceleration for homing sequence. R/W N
#define OD_INTERPOLATED_POSITION_SUB_MODE		0x60C0 // Sets sub-mode for interpolated position algorithm. R/W N
#define OD_INTERPOLATED_DATA_RECORD				0x60C1 // Sets data for interpolation position trajectory. R/W Y
#define OD_INTERPOLATED_POSITION_TIME_PERIOD	0x60C2 // Defines time for interpolation position trajectory. R/W Y
#define OD_INTERPOLATION_DATA_CONFIG			0x60C4 // Defines method to store position data record. R/W Y: buffer position N: all the other entries.
//#define OD_POSITION_DEMAND_VALUE				0x60FC // Reads position command in increments as given to position controller R Y
#define OD_DIGITAL_INPUT						0x60FD // Reads digital input according to DSP 402, and also reflects Elmo digital input logical state. R Y
#define OD_TARGET_VELOCITY						0x60FF // Sets velocity reference for velocity profiler. R/W Y
#define OD_MOTOR_TYPE							0x6402 // R/W N
#define OD_MOTOR_CATALOG_NUMBER					0x6403 // 32 characters. R/W N
#define OD_MOTOR_MANUFACTURER					0x6404 // 32 characters. R/W N
#define OD_HTTP_MOTOR_CATALOG_ADDRESS			0x6405 // R/W N
#define OD_MOTOR_CALIBRATION_DATE				0x6406 // R/W N
#define OD_MOTOR_SERVICE_PERIOD					0x6407 // R/W N
#define OD_DRIVE_MODES							0x6502 // R/W N
#define OD_DRIVE_MANUFACTURER					0x6504 // R N
#define OD_DRIVE_MANUFACTURER_WEB_SITE			0x6505 // R N

////////////////////////////////////////////////
//  MACRO to get function code, COB-ID, Node-ID:
#define COB_ID(fn_code, node_id) ((unsigned short)(fn_code<<7 | node_id))
#define COB_FN_CODE(cob_id) ((cob_id>>7) & 0x0f)
#define COB_NODE_ID(cob_id) (cob_id & 0x7f)

////////////////////////////////////////////////
//  Define LSS Modes:
#define LSS_MODE_CONFIGURATION		0x01
#define LSS_MODE_OPERATION			0x00

////////////////////////////////////////////////
//  Define NMT Communication States:
#define NMT_INITIALIZATION			0x00
#define NMT_PRE_OPERATIONAL			0x01
#define NMT_OPERATIONAL				0x02
#define NMT_STOPPED					0x03

////////////////////////////////////////////////
//  Define NMT Command Specifier:
#define NMT_CMD_NODE_START			0x01
#define NMT_CMD_NODE_STOP			0x02
#define NMT_CMD_NODE_READY			0X80
#define NMT_CMD_NODE_RESET			0x81
#define NMT_CMD_COMM_RESET			0x82

////////////////////////////////////////////////
//  Define Unit Mode:
#define UM_TORQUE					0x01
#define UM_SPEED					0x02
#define UM_MICRO_STEPPER			0x03
#define UM_POSITION_DUAL_FEEDBACK	0x04
#define UM_POSITION					0x05

////////////////////////////////////////////////
//  Define Mode of operation(INT8):
#define OP_MODE_NO_MODE				(-1)
#define OP_MODE_PROFILED_POSITION	(1)
#define OP_MODE_PROFILED_VELOCITY	(3)
#define OP_MODE_PROFILED_TORQUE		(4)
#define OP_MODE_HOMING				(6)
#define OP_MODE_INTERPOLATED_POSITION (7)

////////////////////////////////////////////////
//  Utils:
#ifndef LOBYTE
#define MAKEWORD(a, b)      ((unsigned short)(((unsigned char)(((unsigned int)(a)) & 0xff)) | ((unsigned short)((unsigned char)(((unsigned int)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((int)(((unsigned short)(((unsigned int)(a)) & 0xffff)) | ((unsigned int)((unsigned short)(((unsigned int)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((unsigned short)(((unsigned int)(l)) & 0xffff))
#define HIWORD(l)           ((unsigned short)((((unsigned int)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((unsigned char)(((unsigned int)(w)) & 0xff))
#define HIBYTE(w)           ((unsigned char)((((unsigned int)(w)) >> 8) & 0xff))
#endif


////////////////////////////////////////////////

CANAPI_END

#endif
