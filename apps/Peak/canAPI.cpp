

/*======================*/
/*       Includes       */
/*======================*/
//system headers
#include <stdio.h>
#include <errno.h>
#ifndef _WIN32
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <malloc.h>
#include <assert.h>
//project headers
extern "C" {
#include "Peak/PCANBasic.h"
}
#include "canDef.h"
#include "canAPI.h"


CANAPI_BEGIN

/*=====================*/
/*       Defines       */
/*=====================*/
//macros
#define isAlpha(c) ( ((c >= 'A') && (c <= 'Z')) ? 1 : 0 )
#define isSpace(c) ( (c == ' ') ? 1 : 0 )
#define isDigit(c) ( ((c >= '0') && (c <= '9')) ? 1 : 0 )
#define ADDR2NODE(x) ((((x) >> 5) & 0x001F) - BASE_ID)
#define NODE2ADDR(x) (((mbxID + BASE_ID) << 5) | ((x) + BASE_ID))
#define GROUPID(n)   (((mbxID + BASE_ID) << 5) | (0x0400 + (n)))
#define BROADCAST    (GROUPID(0))
#define Border(Value,Min,Max)  (Value<Min)?Min:((Value>Max)?Max:Value)
//typedefs & structs
typedef unsigned long DWORD;

/*=========================================*/
/*       Global file-scope variables       */
/*=========================================*/
TPCANHandle canDev[MAX_BUS] = {
	PCAN_NONEBUS, // Undefined/default value for a PCAN bus

	PCAN_ISABUS1, // PCAN-ISA interface, channel 1
	PCAN_ISABUS2, // PCAN-ISA interface, channel 2
	PCAN_ISABUS3, // PCAN-ISA interface, channel 3
	PCAN_ISABUS4, // PCAN-ISA interface, channel 4
	PCAN_ISABUS5, // PCAN-ISA interface, channel 5
	PCAN_ISABUS6, // PCAN-ISA interface, channel 6
	PCAN_ISABUS7, // PCAN-ISA interface, channel 7
	PCAN_ISABUS8, // PCAN-ISA interface, channel 8

	PCAN_DNGBUS1, // PCAN-Dongle/LPT interface, channel 1

	PCAN_PCIBUS1, // PCAN-PCI interface, channel 1
	PCAN_PCIBUS2, // PCAN-PCI interface, channel 2
	PCAN_PCIBUS3, // PCAN-PCI interface, channel 3
	PCAN_PCIBUS4, // PCAN-PCI interface, channel 4
	PCAN_PCIBUS5, // PCAN-PCI interface, channel 5
	PCAN_PCIBUS6, // PCAN-PCI interface, channel 6
	PCAN_PCIBUS7, // PCAN-PCI interface, channel 7
	PCAN_PCIBUS8, // PCAN-PCI interface, channel 8

	PCAN_USBBUS1, // PCAN-USB interface, channel 1
	PCAN_USBBUS2, // PCAN-USB interface, channel 2
	PCAN_USBBUS3, // PCAN-USB interface, channel 3
	PCAN_USBBUS4, // PCAN-USB interface, channel 4
	PCAN_USBBUS5, // PCAN-USB interface, channel 5
	PCAN_USBBUS6, // PCAN-USB interface, channel 6
	PCAN_USBBUS7, // PCAN-USB interface, channel 7
	PCAN_USBBUS8, // PCAN-USB interface, channel 8

	PCAN_PCCBUS1, // PCAN-PC Card interface, channel 1
	PCAN_PCCBUS2, // PCAN-PC Card interface, channel 2
};

/*==========================================*/
/* Get channel index for Peak CAN interface */
/*==========================================*/
int getPCANChannelIndex(const char* cname)
{
	if (!cname) return 0;

	if (!_stricmp(cname, "0") || !_stricmp(cname, "PCAN_NONEBUS") || !_stricmp(cname, "NONEBUS"))
		return 0;
	else if (!_stricmp(cname, "1") || !_stricmp(cname, "PCAN_ISABUS1") || !_stricmp(cname, "ISABUS1"))
		return 1;
	else if (!_stricmp(cname, "2") || !_stricmp(cname, "PCAN_ISABUS2") || !_stricmp(cname, "ISABUS2"))
		return 2;
	else if (!_stricmp(cname, "3") || !_stricmp(cname, "PCAN_ISABUS3") || !_stricmp(cname, "ISABUS3"))
		return 3;
	else if (!_stricmp(cname, "4") || !_stricmp(cname, "PCAN_ISABUS4") || !_stricmp(cname, "ISABUS4"))
		return 4;
	else if (!_stricmp(cname, "5") || !_stricmp(cname, "PCAN_ISABUS5") || !_stricmp(cname, "ISABUS5"))
		return 5;
	else if (!_stricmp(cname, "6") || !_stricmp(cname, "PCAN_ISABUS6") || !_stricmp(cname, "ISABUS6"))
		return 6;
	else if (!_stricmp(cname, "7") || !_stricmp(cname, "PCAN_ISABUS7") || !_stricmp(cname, "ISABUS7"))
		return 7;
	else if (!_stricmp(cname, "8") || !_stricmp(cname, "PCAN_ISABUS8") || !_stricmp(cname, "ISABUS8"))
		return 8;
	else if (!_stricmp(cname, "9") || !_stricmp(cname, "PCAN_DNGBUS1") || !_stricmp(cname, "DNGBUS1"))
		return 9;
	else if (!_stricmp(cname, "10") || !_stricmp(cname, "PCAN_PCIBUS1") || !_stricmp(cname, "PCIBUS1"))
		return 10;
	else if (!_stricmp(cname, "11") || !_stricmp(cname, "PCAN_PCIBUS2") || !_stricmp(cname, "PCIBUS2"))
		return 11;
	else if (!_stricmp(cname, "12") || !_stricmp(cname, "PCAN_PCIBUS3") || !_stricmp(cname, "PCIBUS3"))
		return 12;
	else if (!_stricmp(cname, "13") || !_stricmp(cname, "PCAN_PCIBUS4") || !_stricmp(cname, "PCIBUS4"))
		return 13;
	else if (!_stricmp(cname, "14") || !_stricmp(cname, "PCAN_PCIBUS5") || !_stricmp(cname, "PCIBUS5"))
		return 14;
	else if (!_stricmp(cname, "15") || !_stricmp(cname, "PCAN_PCIBUS6") || !_stricmp(cname, "PCIBUS6"))
		return 15;
	else if (!_stricmp(cname, "16") || !_stricmp(cname, "PCAN_PCIBUS7") || !_stricmp(cname, "PCIBUS7"))
		return 16;
	else if (!_stricmp(cname, "17") || !_stricmp(cname, "PCAN_PCIBUS8") || !_stricmp(cname, "PCIBUS8"))
		return 17;
	else if (!_stricmp(cname, "18") || !_stricmp(cname, "PCAN_USBBUS1") || !_stricmp(cname, "USBBUS1"))
		return 18;
	else if (!_stricmp(cname, "19") || !_stricmp(cname, "PCAN_USBBUS2") || !_stricmp(cname, "USBBUS2"))
		return 19;
	else if (!_stricmp(cname, "20") || !_stricmp(cname, "PCAN_USBBUS3") || !_stricmp(cname, "USBBUS3"))
		return 20;
	else if (!_stricmp(cname, "21") || !_stricmp(cname, "PCAN_USBBUS4") || !_stricmp(cname, "USBBUS4"))
		return 21;
	else if (!_stricmp(cname, "22") || !_stricmp(cname, "PCAN_USBBUS5") || !_stricmp(cname, "USBBUS5"))
		return 22;
	else if (!_stricmp(cname, "23") || !_stricmp(cname, "PCAN_USBBUS6") || !_stricmp(cname, "USBBUS6"))
		return 23;
	else if (!_stricmp(cname, "24") || !_stricmp(cname, "PCAN_USBBUS7") || !_stricmp(cname, "USBBUS7"))
		return 24;
	else if (!_stricmp(cname, "25") || !_stricmp(cname, "PCAN_USBBUS8") || !_stricmp(cname, "USBBUS8"))
		return 25;
	else if (!_stricmp(cname, "26") || !_stricmp(cname, "PCAN_PCCBUS1") || !_stricmp(cname, "PCCBUS1"))
		return 26;
	else if (!_stricmp(cname, "27") || !_stricmp(cname, "PCAN_PCCBUS2") || !_stricmp(cname, "PCCBUS2"))
		return 271;
	else
		return 0;
}

/*==========================================*/
/*       Private functions prototypes       */
/*==========================================*/
int canReadMsg(int bus, unsigned long& id, unsigned char& len, unsigned char *data, bool blocking);
int canSendMsg(int bus, unsigned long id, unsigned char len, unsigned char *data, bool /*blocking*/);

/*========================================*/
/*       Public functions (CAN API)       */
/*========================================*/
int initCAN(int bus){
	TPCANStatus Status = PCAN_ERROR_OK;
	char strMsg[256];
	TPCANBaudrate Baudrate = PCAN_BAUD_1M;
	TPCANType HwType = 0;
	DWORD IOPort = 0;
	WORD Interrupt = 0;

	Status = CAN_Initialize(canDev[bus], Baudrate, HwType, IOPort, Interrupt);
	if (Status != PCAN_ERROR_OK)
	{
		CAN_GetErrorText(Status, 0, strMsg);
		printf("initCAN(): CAN_Initialize() failed with error %ld\n", Status);
		printf("%s\n", strMsg);
		return Status;
	}

	//Status = CAN_FilterMessages(
	//	canDev[bus],
	//	((unsigned long)(ID_CMD_QUERY_CONTROL_DATA) <<6) | ((unsigned long)ID_DEVICE_MAIN <<3) | ((unsigned long)ID_DEVICE_SUB_01),
	//	((unsigned long)(ID_CMD_QUERY_CONTROL_DATA) <<6) | ((unsigned long)ID_DEVICE_MAIN <<3) | ((unsigned long)ID_DEVICE_SUB_04),
	//	PCAN_MESSAGE_STANDARD);
	//if (Status != PCAN_ERROR_OK)
	//{
	//	CAN_GetErrorText(Status, 0, strMsg);
	//	printf("initCAN(): CAN_FilterMessages() failed with error %ld\n", Status);
	//	printf("%s\n", strMsg);
	//	//return Status;
	//}

	Status = CAN_Reset(canDev[bus]);
	if (Status != PCAN_ERROR_OK)
	{
		CAN_GetErrorText(Status, 0, strMsg);
		printf("initCAN(): CAN_Reset() failed with error %ld\n", Status);
		printf("%s\n", strMsg);
		//return Status;
	}

	return 0; // PCAN_ERROR_OK
}

int freeCAN(int bus){
	TPCANStatus Status = PCAN_ERROR_OK;
	char strMsg[256];

	Status = CAN_Uninitialize(canDev[bus]);
	if (Status != PCAN_ERROR_OK)
	{
		CAN_GetErrorText(Status, 0, strMsg);
		printf("freeCAN(): CAN_Uninitialize() failed with error %ld\n", Status);
		printf("%s\n", strMsg);
		return Status;
	}

	return 0; // PCAN_ERROR_OK
}

int canReadMsg(int bus, unsigned long& id, unsigned char& len, unsigned char *data, bool blocking) 
{
	TPCANMsg CANMsg;
	TPCANTimestamp CANTimeStamp;
	TPCANStatus Status = PCAN_ERROR_OK;
	char strMsg[256];
	int i;

	// We execute the "Read" function of the PCANBasic                
	//
	do {

		Status = CAN_Read(canDev[bus], &CANMsg, &CANTimeStamp);
	
		if (Status != PCAN_ERROR_OK) {
			if (Status != PCAN_ERROR_QRCVEMPTY) {
				CAN_GetErrorText(Status, 0, strMsg);
				printf("canReadMsg(): CAN_Read() failed with error %ld\n", Status);
				printf("%s\n", strMsg);
				return Status;
			}
			else {
				Sleep(1);
			}
		}
		else {
			id = CANMsg.ID;
			len = CANMsg.LEN;
			for(i = 0; i < CANMsg.LEN; i++)
				data[i] = CANMsg.DATA[i];
		}

	} while (blocking && Status != PCAN_ERROR_OK);

	return Status;
}

int canSendMsg(int bus, unsigned long id, unsigned char len, unsigned char *data, bool /*blocking*/) 
{
	TPCANMsg CANMsg;
	TPCANStatus Status = PCAN_ERROR_OK;
	char strMsg[256];
	int i;

	CANMsg.ID = id;
	CANMsg.LEN = len & 0x0F;
	for(i = 0; i < len; i++)
        CANMsg.DATA[i] = data[i];
	CANMsg.MSGTYPE = PCAN_MESSAGE_STANDARD;

	Status = CAN_Write(canDev[bus], &CANMsg);
	if (Status != PCAN_ERROR_OK)
	{
		CAN_GetErrorText(Status, 0, strMsg);
		printf("canSendMsg(): CAN_Write() failed with error %ld\n", Status);
		printf("%s\n", strMsg);
		return Status;
	}

	return 0;
}

/*========================================*/
/*       CAN API                          */
/*========================================*/
int can_open(int ch)
{
	assert(ch >= 0 && ch < MAX_BUS);

	DWORD ret;

	printf("<< CAN: Open Channel...\n");
	ret = initCAN(ch);
	if (ret != 0) return ret;
	printf("\t- Ch.%2d (OK)\n", ch);
	printf("\t- Done\n");

	return 0;
}
int can_open_ex(int ch, int type, int index)
{
	return can_open(ch);
}
int can_reset(int ch)
{
	return -1;
}
int can_close(int ch)
{
	assert(ch >= 0 && ch < MAX_BUS);

	TPCANStatus Status = PCAN_ERROR_OK;
	char strMsg[256];
	printf("<< CAN: Close...\n");

	Status = CAN_Uninitialize(canDev[ch]);
	if (Status != PCAN_ERROR_OK)
	{
		CAN_GetErrorText(Status, 0, strMsg);
		printf("freeCAN(): CAN_Uninitialize() failed with error %ld\n", Status);
		printf("%s\n", strMsg);
		return Status;
	}

	printf("\t- Done\n");
	return 0; // PCAN_ERROR_OK
}

int can_query_object(int ch, unsigned char node_id, unsigned short obj_index, unsigned char sub_index, unsigned char& rx_len, unsigned char* rx_data)
{
	assert(ch >= 0 && ch < MAX_BUS);

	unsigned long Txid;
	unsigned char data[8];
	unsigned long Rxid;
	unsigned char rx_fn_code; 
	unsigned char rx_node_id; 
	//unsigned short rx_obj_index;
	//unsigned char rx_sub_index;
	int err;
	
	Txid = COB_ID(COBTYPE_RxSDO , node_id);
	data[0] = (0x02<<5); // Initiate SDO upload service
	data[1] = LOBYTE(obj_index); // Index (LO)
	data[2] = HIBYTE(obj_index); // Index (HI)
	data[3] = sub_index; // Sub-index
	data[4] = 0x00; // reserved. must be 0.
	data[5] = 0x00; // reserved. must be 0.
	data[6] = 0x00; // reserved. must be 0.
	data[7] = 0x00; // reserved. must be 0.

	err = canSendMsg(ch, Txid, 8, data, true);
	if (err) return err;

	do {
		err = canReadMsg(ch, Rxid, rx_len, rx_data, true);
		if (err) return err;
	} while (Rxid != COB_ID(COBTYPE_TxSDO, node_id));

	rx_fn_code = FN_CODE(Rxid);
	rx_node_id = NODE_ID(Rxid);

	printf("    %04xh (fn=%s(%d), node=%d, len=%d)", Rxid, COBTYPE_NAME(rx_fn_code), rx_fn_code, rx_node_id, rx_len);
	for(int nd=0; nd<rx_len; nd++) printf(" %02X ", rx_data[nd]);
	printf("\n");

	return 0;
}

int can_get_message(int ch, 
					unsigned char& fn_code, 
					unsigned char& node_id, 
					unsigned char& len, 
					unsigned char* data, 
					bool blocking)
{
	int err;
	unsigned long Rxid;

	err = canReadMsg(ch, Rxid, len, data, blocking);
	if (err) return err;

	fn_code = FN_CODE(Rxid);
	node_id = NODE_ID(Rxid);

	printf("    %04xh (fn=%s(%d), node=%d, len=%d)", Rxid, COBTYPE_NAME(fn_code), fn_code, node_id, len);
	for(int nd=0; nd<len; nd++) printf(" %02X ", data[nd]);
	printf("\n");
	
	return 0;
}

int can_dump_slave(int ch, unsigned char node_id)
{
	return 0;
}





int can_nmt_change_state(int ch, unsigned char node_id)
{
	return 0;
}
int can_nmt_query_state(int ch, unsigned char node_id)
{
	return 0;
}
int can_sys_init(int ch, unsigned char node_id, int period_msec)
{
	return 0;
}
int can_pdo_map(int ch, unsigned char node_id)
{
	return 0;
}
int can_set_mode_of_operation(int ch, unsigned char node_id, unsigned short opmode)
{
	return 0;
}
int can_servo_on(int ch, unsigned char node_id)
{
	return 0;
}
int can_servo_off(int ch, unsigned char node_id)
{
	return 0;
}

int can_write_PT(int ch, unsigned char node_id, unsigned short position)
{
	assert(ch >= 0 && ch < MAX_BUS);

	/*long Txid;
	unsigned char data[8];
	int ret;

	if (findex >= 0 && findex < 4)
	{
		data[0] = (unsigned char)( (pwm[0] >> 8) & 0x00ff);
		data[1] = (unsigned char)(pwm[0] & 0x00ff);

		data[2] = (unsigned char)( (pwm[1] >> 8) & 0x00ff);
		data[3] = (unsigned char)(pwm[1] & 0x00ff);

		data[4] = (unsigned char)( (pwm[2] >> 8) & 0x00ff);
		data[5] = (unsigned char)(pwm[2] & 0x00ff);

		data[6] = (unsigned char)( (pwm[3] >> 8) & 0x00ff);
		data[7] = (unsigned char)(pwm[3] & 0x00ff);

		Txid = ((unsigned long)(ID_CMD_SET_TORQUE_1 + findex)<<6) | ((unsigned long)ID_COMMON <<3) | ((unsigned long)ID_DEVICE_MAIN);
		ret = canSendMsg(ch, Txid, 8, data, TRUE);
	}
	else
		return -1;*/
	
	return 0;
}

int can_store_params(int ch, unsigned char node_id)
{
	return 0;
}
int can_restore_params(int ch, unsigned char node_id)
{
	return 0;
}

int can_query_device_type(int ch, unsigned char node_id)
{
	unsigned char len;
	unsigned char data[8];
	int err;
	
	err = can_query_object(ch, node_id, OD_DEVICE_TYPE, 0, len, data);

	return err;
}

int can_query_device_name(int ch, unsigned char node_id)
{
	unsigned char len;
	unsigned char data[8];
	int err;
	
	err = can_query_object(ch, node_id, OD_DEVICE_NAME, 0, len, data);

	return err;
}

int can_query_hw_version(int ch, unsigned char node_id)
{
	unsigned char len;
	unsigned char data[8];
	int err;
	
	err = can_query_object(ch, node_id, OD_HW_VERSION, 0, len, data);

	return err;
}

int can_query_sw_version(int ch, unsigned char node_id)
{
	unsigned char len;
	unsigned char data[8];
	int err;
	
	err = can_query_object(ch, node_id, OD_SW_VERSION, 0, len, data);

	return err;
}

int can_query_node_id(int ch, unsigned char node_id)
{
	unsigned char len;
	unsigned char data[8];
	int err;
	
	err = can_query_object(ch, node_id, OD_NODEID, 1, len, data);

	return err;
}

int can_query_RxPDO_mapping(int ch, unsigned char node_id, unsigned char pdo_id)
{
	unsigned char len;
	unsigned char data[8];
	int err;

	unsigned short obj_index = 
		(pdo_id == 1 ? OD_RxPDO1_MAPPING :
		(pdo_id == 2 ? OD_RxPDO2_MAPPING :
		(pdo_id == 3 ? OD_RxPDO3_MAPPING :
		(pdo_id == 4 ? OD_RxPDO4_MAPPING : 0))));
	if (!obj_index) return -1;

	for (int sub_index=1; sub_index<=8; sub_index++) {
		err = can_query_object(ch, node_id, obj_index, sub_index, len, data);
		if (err) return err;
	}
	return 0;
}

int can_query_TxPDO_mapping(int ch, unsigned char node_id, unsigned char pdo_id)
{
	unsigned char len;
	unsigned char data[8];
	int err;
	
	unsigned short obj_index = 
		(pdo_id == 1 ? OD_TxPDO1_MAPPING :
		(pdo_id == 2 ? OD_TxPDO2_MAPPING :
		(pdo_id == 3 ? OD_TxPDO3_MAPPING :
		(pdo_id == 4 ? OD_TxPDO4_MAPPING : 0))));
	if (!obj_index) return -1;

	for (int sub_index=1; sub_index<=8; sub_index++) {
		err = can_query_object(ch, node_id, obj_index, sub_index, len, data);
		if (err) return err;
	}
	return 0;
}

int can_query_lss_address(int ch, unsigned char node_id)
{
	unsigned char len;
	unsigned char data[8];
	int err;
	
	err = can_query_object(ch, node_id, OD_LSS_ADDRESS, 1, len, data); // vendor ID (unsigned32)
	if (err) return err;
	err = can_query_object(ch, node_id, OD_LSS_ADDRESS, 2, len, data); // product ID (unsigned32)
	if (err) return err;
	err = can_query_object(ch, node_id, OD_LSS_ADDRESS, 3, len, data); // revision number (unsigned32)
	if (err) return err;
	err = can_query_object(ch, node_id, OD_LSS_ADDRESS, 4, len, data); // serial number (unsigned 32)
	if (err) return err;

	return 0;
}

int can_query_position(int ch, unsigned char node_id)
{
	return 0;
}

int can_lss_switch_mode(int ch, unsigned char node_id, unsigned char mode)
{
	assert(ch >= 0 && ch < MAX_BUS);

	long Txid;
	unsigned char data[8];
	int ret;

	if (NODEID_GLOBAL == node_id) {
		data[0] = (unsigned char)(0x04);
		data[1] = (unsigned char)(mode);
	}
	else {
		data[0] = (unsigned char)(0x64);
		data[1] = (unsigned char)(mode);
	}

	Txid = COBID_LSS_REQ;
	ret = canSendMsg(ch, Txid, 8, data, TRUE);
	
	return 0;
}



CANAPI_END
