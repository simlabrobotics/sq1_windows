

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
#ifdef CAN_PRINT_DIAGNOSIS
	printf("<< CAN: Open Channel...\n");
#endif
	ret = initCAN(ch);
	if (ret != 0) return ret;
#ifdef CAN_PRINT_DIAGNOSIS
	printf("\t- Ch.%2d (OK)\n", ch);
	printf("\t- Done\n");
#endif
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
#ifdef CAN_PRINT_DIAGNOSIS
	printf("<< CAN: Close...\n");
#endif
	Status = CAN_Uninitialize(canDev[ch]);
	if (Status != PCAN_ERROR_OK)
	{
		CAN_GetErrorText(Status, 0, strMsg);
		printf("freeCAN(): CAN_Uninitialize() failed with error %ld\n", Status);
		printf("%s\n", strMsg);
		return Status;
	}
#ifdef CAN_PRINT_DIAGNOSIS
	printf("\t- Done\n");
#endif
	return 0; // PCAN_ERROR_OK
}

int can_nmt_node_start(int ch, unsigned char node_id)
{
	assert(ch >= 0 && ch < MAX_BUS);

	long Txid;
	unsigned char data[8];
	int ret;

	data[0] = (unsigned char)(NMT_CMD_NODE_START);
	data[1] = (unsigned char)(node_id);

	Txid = COBID_NMT;
	ret = canSendMsg(ch, Txid, 2, data, true);
	
	return 0;
}

int can_nmt_node_stop(int ch, unsigned char node_id)
{
	assert(ch >= 0 && ch < MAX_BUS);

	long Txid;
	unsigned char data[8];
	int ret;

	data[0] = (unsigned char)(NMT_CMD_NODE_STOP);
	data[1] = (unsigned char)(node_id);

	Txid = COBID_NMT;
	ret = canSendMsg(ch, Txid, 2, data, true);
	
	return 0;
}

int can_nmt_node_ready(int ch, unsigned char node_id)
{
	assert(ch >= 0 && ch < MAX_BUS);

	long Txid;
	unsigned char data[8];
	int ret;

	data[0] = (unsigned char)(NMT_CMD_NODE_READY);
	data[1] = (unsigned char)(node_id);

	Txid = COBID_NMT;
	ret = canSendMsg(ch, Txid, 2, data, true);
	
	return 0;
}

int can_nmt_soft_reset(int ch, unsigned char node_id)
{
	assert(ch >= 0 && ch < MAX_BUS);

	long Txid;
	unsigned char data[8];
	int ret;

	data[0] = (unsigned char)(NMT_CMD_NODE_RESET);
	data[1] = (unsigned char)(node_id);

	Txid = COBID_NMT;
	ret = canSendMsg(ch, Txid, 2, data, true);
	
	return 0;
}

int can_nmt_hard_reset(int ch, unsigned char node_id)
{
	/*assert(ch >= 0 && ch < MAX_BUS);

	long Txid;
	unsigned char data[8];
	int ret;

	data[0] = (unsigned char)(NMT_CMD_COMM_RESET);
	data[1] = (unsigned char)(node_id);

	Txid = COBID_NMT;
	ret = canSendMsg(ch, Txid, 2, data, true);
	
	return 0;*/
	return -1;
}

int can_sync(int ch)
{
	assert(ch >= 0 && ch < MAX_BUS);

	long Txid;
	int ret;

	Txid = COBID_SYNC;
	ret = canSendMsg(ch, Txid, 0, 0, true);
	
	return 0;
}

int can_timestamp(int ch)
{
	/*assert(ch >= 0 && ch < MAX_BUS);

	long Txid;
	unsigned char data[8];
	int ret;

	data[0] = 0x00;
	data[1] = 0x00;

	Txid = COBID_TIMESTAMP;
	ret = canSendMsg(ch, Txid, 0, data, true);
	
	return 0;*/
	return -1;
}



int can_sdo_request(int ch, unsigned char node_id, unsigned short obj_index, unsigned char sub_index, unsigned char* buf, unsigned short& buf_len)
{
	assert(ch >= 0 && ch < MAX_BUS);
	assert(buf && buf_len > 0);

#pragma pack(push)
#pragma pack(1)
	typedef struct {
		union {
			unsigned char d[8];
			struct {
				union {
					// initiate SDO upload protocol (client -> server):
					struct {
						unsigned char x1  : 5; // (not used)
						unsigned char ccs : 3; // client command specifier (client to server)
					};
					// upload SDO segment protocol (client -> server):
					struct {
						unsigned char x2  : 4; // (not used)
						unsigned char t   : 1; // toggle bit
						unsigned char ccs : 3; // client command specifier (client to server)
					};
					// initiate SDO upload protocol (server -> client):
					struct {
						unsigned char s   : 1; // size indicator
						unsigned char e   : 1; // transfer type;
						unsigned char n   : 2; // number of bytes in data
						unsigned char x3  : 1; // (not used)
						unsigned char scs : 3; // server command specifier (server to client)
					};
					// upload SDO segment protocol (server -> client):
					struct {
						unsigned char c   : 1; // more segments to be uploaded(0: more, 1:no more)
						unsigned char x4  : 3; // (not used)
						unsigned char t   : 1; // toggle bit
						unsigned char scs : 3; // server command specifier (server to client)
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
	} TxData;
#pragma pack(pop)

	unsigned long tx_id;
	TxData tx_data;
	unsigned long rx_id;
	TxData rx_data;
	unsigned char rx_len;
#ifdef CAN_PRINT_Rx_MESSAGE
	unsigned char rx_fn_code; 
	unsigned char rx_node_id; 
#endif
	int err;
	unsigned short buf_size;
	int rx_seg_data_index;

	buf_size = buf_len;
	buf_len = 0;

	tx_id = COB_ID(COBTYPE_RxSDO , node_id);

	// initiate SDO upload protocol
	tx_data[0] = 0x00;
	tx_data.ccs = 2;
	tx_data.obj_index = obj_index;
	tx_data.sub_index = sub_index;
	tx_data[4] = 0x00; // reserved. must be 0.
	tx_data[5] = 0x00; // reserved. must be 0.
	tx_data[6] = 0x00; // reserved. must be 0.
	tx_data[7] = 0x00; // reserved. must be 0.

	do {
		err = canSendMsg(ch, tx_id, 8, tx_data, true);
		if (err) return err;

		do {
			err = canReadMsg(ch, rx_id, rx_len, rx_data, true);
			if (err) return err;
		} while (rx_id != COB_ID(COBTYPE_TxSDO, node_id));

#ifdef CAN_PRINT_Rx_MESSAGE
		rx_fn_code = FN_CODE(rx_id);
		rx_node_id = NODE_ID(rx_id);
		printf("    %04xh (fn=%s(%d), node=%d, len=%d)", rx_id, COBTYPE_NAME(rx_fn_code), rx_fn_code, rx_node_id, rx_len);
		for(int nd=0; nd<rx_len; nd++) printf(" %02X ", rx_data[nd]);
		printf("\n");
#endif

		if (rx_data.scs == 2) {
			if (rx_data.e == 1) {
				for (rx_seg_data_index = 4; rx_seg_data_index < (8-rx_data.n); rx_seg_data_index++)
					buf[buf_len++] = rx_data[rx_seg_data_index];
				break; // expedited transfer
			}
			else {
				// prepare upload SDO segment request:
				tx_data.ccs = 3; // upload segment request
				tx_data.t = 0; // toggle bit
			}
		}
		else if (rx_data.scs == 0) {
			for (rx_seg_data_index = 1; rx_seg_data_index < (8-rx_data.n); rx_seg_data_index++)
				buf[buf_len++] = rx_data[rx_seg_data_index];

			if (rx_data.c == 1) {
				break; // no more segment response
			}
			else {
				tx_data.t = (tx_data.t == 0 ? 1 : 0); // toggle
			}
		}
		else if (rx_data.scs == 4) {
			printf("<< upload SDO transaction aborted.(error code = %04X %04X)\n", MAKEWORD(rx_data[6], rx_data[7]), MAKEWORD(rx_data[4], rx_data[5]));
			return MAKELONG(MAKEWORD(rx_data[4], rx_data[5]), MAKEWORD(rx_data[6], rx_data[7]));
		}
		else {
			// abort transaction:
			printf("<< upload SDO transaction aborted.(abnormal scs code)\n");
			return -1;
		}
	} while (true);

	return 0;
}

int can_query_device_type(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;

	err = can_sdo_request(ch, node_id, OD_DEVICE_TYPE, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tdevice profile number = %d\n", MAKEWORD(buf[2], buf[3]));
		printf("\tnumber of SDOs supported = %d\n", MAKEWORD(buf[0], buf[1]));
	}
#endif
	return err;
}

int can_query_device_name(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	int buf_index;
	
	err = can_sdo_request(ch, node_id, OD_DEVICE_NAME, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	//if (!err) {
		printf("\tdevice name = ");
		for (buf_index = 0; buf_index < buf_len; buf_index++)
			printf("%c", buf[buf_index]);
		printf("\n");
	//}
#endif
	return err;
}

int can_query_hw_version(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	int buf_index;
	
	err = can_sdo_request(ch, node_id, OD_HW_VERSION, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	//if (!err) {
		printf("\tH/W version = ");
		for (buf_index = 0; buf_index < buf_len; buf_index++)
			printf("%c", buf[buf_index]);
		printf("\n");
	//}
#endif
	return err;
}

int can_query_sw_version(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	int buf_index;
	
	err = can_sdo_request(ch, node_id, OD_SW_VERSION, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	//if (!err) {
		printf("\tS/W version = ");
		for (buf_index = 0; buf_index < buf_len; buf_index++)
			printf("%c", buf[buf_index]);
		printf("\n");
	//}
#endif
	return err;
}

int can_query_node_id(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_request(ch, node_id, OD_NODEID, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tnode id = %d\n", buf[0]);
	}
#endif
	return err;
}

int can_query_RxPDO_mapping(int ch, unsigned char node_id, unsigned char pdo_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	unsigned char entry_num; // number of sub-entries

	unsigned short obj_index = 
		(pdo_id == 1 ? OD_RxPDO1_MAPPING :
		(pdo_id == 2 ? OD_RxPDO2_MAPPING :
		(pdo_id == 3 ? OD_RxPDO3_MAPPING :
		(pdo_id == 4 ? OD_RxPDO4_MAPPING : 0))));
	if (!obj_index) return -1;

	err = can_sdo_request(ch, node_id, obj_index, 0, buf, buf_len);
	if (!err) {
		entry_num = buf[2];
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tRxPDO%d mapping.num_of_entries = %d\n", pdo_id, entry_num);
#endif
	}
	else
		return err;

	for (int sub_index=1; sub_index<=entry_num; sub_index++) {
		err = can_sdo_request(ch, node_id, obj_index, sub_index, buf, buf_len);
		if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
			printf("\tRxPDO%d mapping[%d].obj_index = %04Xh\n", pdo_id, sub_index, MAKEWORD(buf[2], buf[3]));
			printf("\tRxPDO%d mapping[%d].sub_index = %d\n", pdo_id, sub_index, buf[1]);
			printf("\tRxPDO%d mapping[%d].obj_length = %d\n", pdo_id, sub_index, buf[0]);
#endif
		}
		else
			return err;
	}
	return 0;
}

int can_query_TxPDO_mapping(int ch, unsigned char node_id, unsigned char pdo_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	unsigned char entry_num; // number of sub-entries
	
	unsigned short obj_index = 
		(pdo_id == 1 ? OD_TxPDO1_MAPPING :
		(pdo_id == 2 ? OD_TxPDO2_MAPPING :
		(pdo_id == 3 ? OD_TxPDO3_MAPPING :
		(pdo_id == 4 ? OD_TxPDO4_MAPPING : 0))));
	if (!obj_index) return -1;

	err = can_sdo_request(ch, node_id, obj_index, 0, buf, buf_len);
	if (!err) {
		entry_num = buf[2];
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tTxPDO%d mapping.num_of_entries = %d\n", pdo_id, entry_num);
#endif
	}
	else
		return err;

	for (int sub_index=1; sub_index<=entry_num; sub_index++) {
		err = can_sdo_request(ch, node_id, obj_index, sub_index, buf, buf_len);
		if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
			printf("\tTxPDO%d mapping[%d].obj_index = %04Xh\n", pdo_id, sub_index, MAKEWORD(buf[2], buf[3]));
			printf("\tTxPDO%d mapping[%d].sub_index = %d\n", pdo_id, sub_index, buf[1]);
			printf("\tTxPDO%d mapping[%d].obj_length = %d\n", pdo_id, sub_index, buf[0]);
#endif
		}
		else
			return err;
	}
	return 0;
}

int can_query_lss_address(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_request(ch, node_id, OD_LSS_ADDRESS, 1, buf, buf_len); // vendor ID (unsigned32)
	if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tvendor id = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
#endif
	}
	else
		return err;

	err = can_sdo_request(ch, node_id, OD_LSS_ADDRESS, 2, buf, buf_len); // product ID (unsigned32)
	if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tproduct id = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
#endif
	}
	else
		return err;

	err = can_sdo_request(ch, node_id, OD_LSS_ADDRESS, 3, buf, buf_len); // revision number (unsigned32)
	if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\trevision id = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
#endif
	}
	else
		return err;

	err = can_sdo_request(ch, node_id, OD_LSS_ADDRESS, 4, buf, buf_len); // serial number (unsigned 32)
	if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tserial id = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
#endif
	}
	else
		return err;

	return 0;
}

int can_query_position(int ch, unsigned char node_id)
{
	return 0;
}

int can_bin_interprete(int ch, unsigned char node_id, unsigned char* buf, unsigned short buf_len)
{
	return 0;
}

int can_os_interprete(int ch, unsigned char node_id, unsigned char* buf, unsigned short buf_len)
{
	// set OS mode:
	// write command:

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
		ret = canSendMsg(ch, Txid, 8, data, true);
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
	ret = canSendMsg(ch, Txid, 8, data, true);
	
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

#ifdef CAN_PRINT_Rx_MESSAGE
	printf("    %04xh (fn=%s(%d), node=%d, len=%d)", Rxid, COBTYPE_NAME(fn_code), fn_code, node_id, len);
	for(int nd=0; nd<len; nd++) printf(" %02X ", data[nd]);
	printf("\n");
#endif
	
	return 0;
}

int can_dump_slave(int ch, unsigned char node_id)
{
	return 0;
}



CANAPI_END
