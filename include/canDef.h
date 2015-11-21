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
#define OD_DIGITAL_INPUT						0x2200
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

////////////////////////////////////////////////
//  MACRO to get function code, COB-ID, Node-ID:
#define COB_ID(fn_code, node_id) ((unsigned short)(fn_code<<7 | node_id))
#define FN_CODE(cob_id) ((cob_id>>7) & 0x0f)
#define NODE_ID(cob_id) (cob_id & 0x7f)

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
