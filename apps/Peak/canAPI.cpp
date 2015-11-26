

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

/*==========================================*/
/*       print data in binary format        */
/*==========================================*/
static void printBinary(unsigned char by)
{
	char sz[9];
	int i;
	sz[0] = '\0';
	for (i=128; i>0; i>>=1)
		printf("%c", (by&i?'1':'0'));
}

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

#ifdef CAN_PRINT_Rx_MESSAGE
		printf("    Rx %04xh (fn=%s(%d), node=%d, len=%d)", id, COBTYPE_NAME(COB_FN_CODE(id)), COB_FN_CODE(id), COB_NODE_ID(id), len);
		for(int nd=0; nd<len; nd++) printf(" %02X ", data[nd]);
		printf("\n");
#endif
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

#ifdef CAN_PRINT_Tx_MESSAGE
	printf("    Tx %04xh (fn=%s(%d), node=%d, len=%d)", id, COBTYPE_NAME(COB_FN_CODE(id)), COB_FN_CODE(id), COB_NODE_ID(id), len);
	for(int nd=0; nd<len; nd++) printf(" %02X ", data[nd]);
	printf("\n");
#endif

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

int can_sdo_download(int ch, unsigned char node_id, unsigned short obj_index, unsigned char sub_index, unsigned char* buf, unsigned short buf_len)
{
	assert(ch >= 0 && ch < MAX_BUS);
	assert(buf && buf_len > 0);

	unsigned long tx_id;
	SDO_data_t tx_data;
	unsigned long rx_id;
	SDO_data_t rx_data;
	unsigned char rx_len;
	int err;
	unsigned short buf_sent;
	int tx_seg_data_index;

	buf_sent = 0;

	tx_id = COB_ID(COBTYPE_RxSDO , node_id);

	// initiate SDO download protocol
	tx_data[0] = 0x00;
	tx_data.cs = SDO_CCS_INITIATE_DOWNLOAD;
	tx_data.e = (buf_len > 4 ? 0 : 1);
	tx_data.s = (buf_len > 4 ? 1 : 0);
	tx_data.n1 = (buf_len > 4 ? 0 : (4 - buf_len));
	tx_data.obj_index = obj_index;
	tx_data.sub_index = sub_index;
	tx_data[4] = 0x00;
	tx_data[5] = 0x00;
	tx_data[6] = 0x00;
	tx_data[7] = 0x00;
	if (tx_data.e == 1) {
		if (tx_data.s == 1) {
			// Data-area contains data of length 4-n to be downloaded. The encoding depends on the type of data referenced by index and sub-index.
			for (; buf_sent < buf_len;)
				tx_data[(buf_sent++) + 4] = buf[buf_sent];
		}
		else {
			// Data-array contains an unspecified number of bytes to be uploaded.
			for (; buf_sent < buf_len;)
				tx_data[(buf_sent++) + 4] = buf[buf_sent];
		}
	}
	else {
		if (tx_data.s == 1) {
			// Data-area contains the number of bytes to be downloaded. Byte 4 contains LSB and byte 7 contains MSB.
			tx_data[4] = LOBYTE(LOWORD(buf_len));
			tx_data[5] = HIBYTE(LOWORD(buf_len));
			tx_data[6] = LOBYTE(HIWORD(buf_len));
			tx_data[7] = HIBYTE(HIWORD(buf_len));
		}
		else {
			// Data-area is reserved for future use.
		}
	}

	do {
		err = canSendMsg(ch, tx_id, 8, tx_data, true);
		if (err) return err;

		do {
			err = canReadMsg(ch, rx_id, rx_len, rx_data, true);
			if (err) return err;
		} while (rx_id != COB_ID(COBTYPE_TxSDO, node_id));

		if (rx_data.cs == SDO_SCS_INITIATE_DOWNLOAD) {
			if (tx_data.e == 1) {
				break; // expedited transfer ends
			}
			else {
				// prepare download SDO segment request:
				tx_data.cs = SDO_CCS_DOWNLOAD; // download segment request
				tx_data.n3 = ((buf_len-buf_sent) > 7 ? 0 : (7 - (buf_len-buf_sent)));
				tx_data.c = ((buf_len-buf_sent) > 7 ? 0 : 1);
				tx_data.t = 0; // toggle bit

				for (tx_seg_data_index = 1; tx_seg_data_index < (8-tx_data.n3); tx_seg_data_index++)
					tx_data[tx_seg_data_index] = buf[buf_sent++];
			}
		}
		else if (rx_data.cs == SDO_SCS_DOWNLOAD) {
			if (tx_data.c == 1) {
				break; // no more segment response
			}
			else {
				tx_data.n3 = ((buf_len-buf_sent) > 7 ? 0 : (7 - (buf_len-buf_sent)));
				tx_data.c = ((buf_len-buf_sent) > 7 ? 0 : 1);
				tx_data.t = (tx_data.t == 0 ? 1 : 0); // toggle

				for (tx_seg_data_index = 1; tx_seg_data_index < (8-tx_data.n3); tx_seg_data_index++)
					tx_data[tx_seg_data_index] = buf[buf_sent++];
			}
		}
		else if (rx_data.cs == SDO_CS_ABORT) {
			printf("<< download SDO transaction aborted.(error code = %04X %04X)\n", MAKEWORD(rx_data[6], rx_data[7]), MAKEWORD(rx_data[4], rx_data[5]));
			return MAKELONG(MAKEWORD(rx_data[4], rx_data[5]), MAKEWORD(rx_data[6], rx_data[7]));
		}
		else {
			// abort transaction:
			printf("<< download SDO transaction aborted.(abnormal scs code)\n");
			return -1;
		}
	} while (true);

	return 0;
}

int can_sdo_upload(int ch, unsigned char node_id, unsigned short obj_index, unsigned char sub_index, unsigned char* buf, unsigned short& buf_len)
{
	assert(ch >= 0 && ch < MAX_BUS);
	assert(buf && buf_len > 0);

	unsigned long tx_id;
	SDO_data_t tx_data;
	unsigned long rx_id;
	SDO_data_t rx_data;
	unsigned char rx_len;
	long rx_len_to_be_uploaded = 0;
	int err;
	unsigned short buf_size;
	int rx_seg_data_index;

	buf_size = buf_len;
	buf_len = 0;

	tx_id = COB_ID(COBTYPE_RxSDO , node_id);

	// initiate SDO upload protocol
	tx_data[0] = 0x00;
	tx_data.cs = SDO_CCS_INITIATE_UPLOAD;
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

		if (rx_data.cs == SDO_SCS_INITIATE_UPLOAD) {
			if (rx_data.e == 1) {
				if (rx_data.s == 1) {
					rx_len_to_be_uploaded = 4-rx_data.n1;
					for (rx_seg_data_index = 4; rx_seg_data_index < (8-rx_data.n1); rx_seg_data_index++)
						buf[buf_len++] = rx_data[rx_seg_data_index];
				}
				else {
					rx_len_to_be_uploaded = 4;
					for (rx_seg_data_index = 4; rx_seg_data_index < 8; rx_seg_data_index++)
						buf[buf_len++] = rx_data[rx_seg_data_index];
				}
				break; // expedited transfer
			}
			else {
				if (rx_data.s == 1) {
					rx_len_to_be_uploaded = MAKELONG(MAKEWORD(rx_data[4], rx_data[5]), MAKEWORD(rx_data[6], rx_data[7]));
				}
				else {
					rx_len_to_be_uploaded = -1;
				}
				// prepare upload SDO segment request:
				tx_data.cs = SDO_CCS_UPLOAD; // upload segment request
				tx_data.t = 0; // toggle bit
			}
		}
		else if (rx_data.cs == SDO_SCS_UPLOAD) {
			for (rx_seg_data_index = 1; rx_seg_data_index < (8-rx_data.n3); rx_seg_data_index++)
				buf[buf_len++] = rx_data[rx_seg_data_index];

			if (rx_data.c == 1) {
				break; // no more segment response
			}
			else {
				tx_data.t = (tx_data.t == 0 ? 1 : 0); // toggle
			}
		}
		else if (rx_data.cs == SDO_CS_ABORT) {
			printf("<< upload SDO transaction aborted.(error code = %04X %04X)\n", MAKEWORD(rx_data[6], rx_data[7]), MAKEWORD(rx_data[4], rx_data[5]));
			return MAKELONG(MAKEWORD(rx_data[4], rx_data[5]), MAKEWORD(rx_data[6], rx_data[7]));
		}
		else {
			// abort transaction:
			printf("<< upload SDO transaction aborted.(abnormal scs code)\n");
			return -1;
		}
	} while (true);

	return ((rx_len_to_be_uploaded < 0 || rx_len_to_be_uploaded == buf_len) ? 0 : -1);
}

int can_query_device_type(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;

	err = can_sdo_upload(ch, node_id, OD_DEVICE_TYPE, 0, buf, buf_len);
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
	
	err = can_sdo_upload(ch, node_id, OD_DEVICE_NAME, 0, buf, buf_len);
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
	
	err = can_sdo_upload(ch, node_id, OD_HW_VERSION, 0, buf, buf_len);
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
	
	err = can_sdo_upload(ch, node_id, OD_SW_VERSION, 0, buf, buf_len);
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
	
	err = can_sdo_upload(ch, node_id, OD_NODEID, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tnode id = %d\n", buf[0]);
	}
#endif
	return err;
}

int can_query_RxPDO_params(int ch, unsigned char node_id, unsigned char pdo_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	unsigned char entry_num; // number of sub-entries

	unsigned short obj_index = 
		(pdo_id == 1 ? OD_RxPDO1_COMM_PARAM :
		(pdo_id == 2 ? OD_RxPDO2_COMM_PARAM :
		(pdo_id == 3 ? OD_RxPDO3_COMM_PARAM :
		(pdo_id == 4 ? OD_RxPDO4_COMM_PARAM : 0))));
	if (!obj_index) return -1;

	/*err = can_sdo_upload(ch, node_id, obj_index, 0, buf, buf_len);
	if (!err) {
		entry_num = buf[2];
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tRxPDO%d communication parameters.num_of_entries = %d\n", pdo_id, entry_num);
#endif
	}
	else
		return err;*/
	entry_num = 2;
	static const char* entry_name[] = {
		"Number of entries",
		"COB-ID used by PDO",
		"Transmission type"
	};

	for (int sub_index=1; sub_index<=2; sub_index++) {
		err = can_sdo_upload(ch, node_id, obj_index, sub_index, buf, buf_len);
		if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
			printf("\tRxPDO%d communication param[%s] = %04Xh %04Xh\n", pdo_id, entry_name[sub_index], MAKEWORD(buf[2], buf[3]), MAKEWORD(buf[0], buf[1]));
#endif
		}
		else
			return err;
	}
	return 0;
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

	err = can_sdo_upload(ch, node_id, obj_index, 0, buf, buf_len);
	if (!err) {
		entry_num = buf[0];
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tRxPDO%d mapping.num_of_entries = %d\n", pdo_id, entry_num);
#endif
	}
	else
		return err;

	for (int sub_index=1; sub_index<=entry_num; sub_index++) {
		err = can_sdo_upload(ch, node_id, obj_index, sub_index, buf, buf_len);
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

int can_query_TxPDO_params(int ch, unsigned char node_id, unsigned char pdo_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	unsigned char entry_num; // number of sub-entries

	unsigned short obj_index = 
		(pdo_id == 1 ? OD_TxPDO1_COMM_PARAM :
		(pdo_id == 2 ? OD_TxPDO2_COMM_PARAM :
		(pdo_id == 3 ? OD_TxPDO3_COMM_PARAM :
		(pdo_id == 4 ? OD_TxPDO4_COMM_PARAM : 0))));
	if (!obj_index) return -1;

	/*err = can_sdo_upload(ch, node_id, obj_index, 0, buf, buf_len);
	if (!err) {
		entry_num = buf[2];
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tTxPDO%d communication parameters.num_of_entries = %d\n", pdo_id, entry_num);
#endif
	}
	else
		return err;*/
	entry_num = 5;
	static const char* entry_name[] = {
		"Number of entries",
		"COB-ID used by PDO",
		"Transmission type",
		"Inhibit time",
		"reserved",
		"Event timer"
	};

	for (int sub_index=1; sub_index<=entry_num; sub_index++) {
		if (sub_index == 4) continue;

		err = can_sdo_upload(ch, node_id, obj_index, sub_index, buf, buf_len);
		if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
			printf("\tTxPDO%d communication param[%s] = %04Xh %04Xh\n", pdo_id, entry_name[sub_index], MAKEWORD(buf[2], buf[3]), MAKEWORD(buf[0], buf[1]));
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

	err = can_sdo_upload(ch, node_id, obj_index, 0, buf, buf_len);
	if (!err) {
		entry_num = buf[0];
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tTxPDO%d mapping.num_of_entries = %d\n", pdo_id, entry_num);
#endif
	}
	else
		return err;

	for (int sub_index=1; sub_index<=entry_num; sub_index++) {
		err = can_sdo_upload(ch, node_id, obj_index, sub_index, buf, buf_len);
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
	
	err = can_sdo_upload(ch, node_id, OD_LSS_ADDRESS, 1, buf, buf_len); // vendor ID (unsigned32)
	if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tvendor id = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
#endif
	}
	else
		return err;

	err = can_sdo_upload(ch, node_id, OD_LSS_ADDRESS, 2, buf, buf_len); // product ID (unsigned32)
	if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\tproduct id = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
#endif
	}
	else
		return err;

	err = can_sdo_upload(ch, node_id, OD_LSS_ADDRESS, 3, buf, buf_len); // revision number (unsigned32)
	if (!err) {
#ifdef CAN_PRINT_SDO_RESPONSE
		printf("\trevision id = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
#endif
	}
	else
		return err;

	err = can_sdo_upload(ch, node_id, OD_LSS_ADDRESS, 4, buf, buf_len); // serial number (unsigned 32)
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

int can_query_control_word(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_upload(ch, node_id, OD_CONTROLWORD, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tcontrol word = ");
		printBinary(buf[1]);
		printf(" ");
		printBinary(buf[0]);
		printf("b\n");
	}
#endif
	return err;
}

int can_query_status_word(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_upload(ch, node_id, OD_STATUSWORD, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tstatus word = ");
		printBinary(buf[1]);
		printf(" ");
		printBinary(buf[0]);
		printf("b\n");
	}
#endif
	return err;
}

int can_query_mode_of_operation_display(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_upload(ch, node_id, OD_MODE_OF_OPERATION_DISPLAY, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tmode of operation display = ");
		switch ((char)(buf[0]))
		{
		case OP_MODE_NO_MODE: printf("not set"); break;
		case OP_MODE_PROFILED_POSITION: printf("Profiled position"); break;
		case OP_MODE_PROFILED_VELOCITY: printf("Profiled velocity"); break;
		case OP_MODE_PROFILED_TORQUE: printf("Profiled torque"); break;
		case OP_MODE_HOMING: printf("Homing"); break;
		case OP_MODE_INTERPOLATED_POSITION: printf("Interpolated position"); break;
		default: printf("Un-known"); break;
		}
		printf("\n");
	}
#endif
	return err;
}

int can_query_drive_modes(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_upload(ch, node_id, OD_DRIVE_MODES, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tsupported drive modes = ");
		printBinary(buf[3]);
		printf(" ");
		printBinary(buf[2]);
		printf(" ");
		printBinary(buf[1]);
		printf(" ");
		printBinary(buf[0]);
		printf("b\n");
	}
#endif
	return err;
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
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	unsigned char entry_num;

	//////////////////////////////////////////////////////
	// mapping TxPDO1:
	entry_num = 0;
	// stop all emissions of TPDO1:
	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf_len = 1;
	err = can_sdo_download(ch, node_id, OD_TxPDO1_MAPPING, 0, buf, buf_len);
	if (err) return err;

	// map position actual value as the 1st 4 bytes of the PDO:
	buf[0] = 32;
	buf[1] = 0;
	buf[2] = LOBYTE(OD_POSITION_ACTUAL_VALUE);
	buf[3] = HIBYTE(OD_POSITION_ACTUAL_VALUE);
	buf_len = 4;
	err = can_sdo_download(ch, node_id, OD_TxPDO1_MAPPING, (++entry_num), buf, buf_len);
	if (err) return err;

	// map digital input as the next 4 bytes of the PDO:
	/*buf[0] = 32;
	buf[1] = 0;
	buf[2] = LOBYTE(OD_DIGITAL_INPUT);
	buf[3] = HIBYTE(OD_DIGITAL_INPUT);
	buf_len = 4;
	err = can_sdo_download(ch, node_id, OD_TxPDO1_MAPPING, (++entry_num), buf, buf_len);
	if (err) return err;*/

	// map status word as the next 2 bytes of the PDO:
	buf[0] = 16;
	buf[1] = 0;
	buf[2] = LOBYTE(OD_STATUSWORD);
	buf[3] = HIBYTE(OD_STATUSWORD);
	buf_len = 4;
	err = can_sdo_download(ch, node_id, OD_TxPDO1_MAPPING, (++entry_num), buf, buf_len);
	if (err) return err;

	// set transmission type in PDO communication parameters to "Transmit every SYNC":
	buf[0] = 1;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf_len = 1;
	err = can_sdo_download(ch, node_id, OD_TxPDO1_COMM_PARAM, 2, buf, buf_len);
	if (err) return err;

	// activate the mapped objects:
	buf[0] = entry_num;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf_len = 1;
	err = can_sdo_download(ch, node_id, OD_TxPDO1_MAPPING, 0, buf, buf_len);
	if (err) return err;

	//////////////////////////////////////////////////////
	// mapping RxPDO1:
	entry_num = 0;
	// stop receiving of RPDO1:
	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf_len = 1;
	err = can_sdo_download(ch, node_id, OD_RxPDO1_MAPPING, 0, buf, buf_len);
	if (err) return err;

	// map profile target position as the 1st 4 bytes of the PDO:
	buf[0] = 32;
	buf[1] = 0;
	buf[2] = LOBYTE(OD_PROFILED_TARGET_POSITION);
	buf[3] = HIBYTE(OD_PROFILED_TARGET_POSITION);
	buf_len = 4;
	err = can_sdo_download(ch, node_id, OD_RxPDO1_MAPPING, (++entry_num), buf, buf_len);
	if (err) return err;

	// map mode of operation as the next 1 byte of the PDO:
	buf[0] = 8;
	buf[1] = 0;
	buf[2] = LOBYTE(OD_MODE_OF_OPERATION);
	buf[3] = HIBYTE(OD_MODE_OF_OPERATION);
	buf_len = 4;
	err = can_sdo_download(ch, node_id, OD_RxPDO1_MAPPING, (++entry_num), buf, buf_len);
	if (err) return err;

	// map control word as the next 2 bytes of the PDO:
	buf[0] = 16;
	buf[1] = 0;
	buf[2] = LOBYTE(OD_CONTROLWORD);
	buf[3] = HIBYTE(OD_CONTROLWORD);
	buf_len = 4;
	err = can_sdo_download(ch, node_id, OD_RxPDO1_MAPPING, (++entry_num), buf, buf_len);
	if (err) return err;

	// set transmission type in PDO communication parameters to "Transmit every SYNC":
	buf[0] = 1;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf_len = 1;
	err = can_sdo_download(ch, node_id, OD_RxPDO1_COMM_PARAM, 2, buf, buf_len);
	if (err) return err;

	// activate the mapped objects:
	buf[0] = entry_num;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf_len = 1;
	err = can_sdo_download(ch, node_id, OD_RxPDO1_MAPPING, 0, buf, buf_len);
	if (err) return err;

	return 0;
}

int can_servo_on(int ch, unsigned char node_id, unsigned short& control_word)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	unsigned short control_word_old;

	// to READY TO SWITCH ON:
	printf("\tto READY TO SWITCH ON...\n");
	control_word_old = control_word;
	control_word &= 0xFF78; // masking irrelevant bits
	control_word |= 0x06;
	buf[0] = LOBYTE(control_word);
	buf[1] = HIBYTE(control_word);
	buf_len = 2;
	err = can_sdo_download(ch, node_id, OD_CONTROLWORD, 0, buf, buf_len);
	if (err) {
		control_word = control_word_old;
		return err;
	}
	Sleep(500);

	// to SWITCHED ON:
	printf("\tto SWITCHED ON...\n");
	control_word_old = control_word;
	control_word &= 0xFF70; // masking irrelevant bits
	control_word |= 0x07;
	buf[0] = LOBYTE(control_word);
	buf[1] = HIBYTE(control_word);
	buf_len = 2;
	err = can_sdo_download(ch, node_id, OD_CONTROLWORD, 0, buf, buf_len);
	if (err) {
		control_word = control_word_old;
		return err;
	}
	Sleep(500);

	// to OPERATION ENABLED:
	printf("\tto OPERATION ENABLED...\n");
	control_word_old = control_word;
	control_word &= 0xFF70; // masking irrelevant bits
	control_word |= 0x0F;
	buf[0] = LOBYTE(control_word);
	buf[1] = HIBYTE(control_word);
	buf_len = 2;
	err = can_sdo_download(ch, node_id, OD_CONTROLWORD, 0, buf, buf_len);
	if (err) {
		control_word = control_word_old;
		return err;
	}
	Sleep(500);

	return 0;
}

int can_servo_off(int ch, unsigned char node_id, unsigned short& control_word)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	unsigned short control_word_old = control_word;

	control_word &= 0xFF7D; // masking irrelevant bits
	control_word |= 0x00;
	buf[0] = LOBYTE(control_word);
	buf[1] = HIBYTE(control_word);
	buf_len = 2;
	err = can_sdo_download(ch, node_id, OD_CONTROLWORD, 0, buf, buf_len);
	if (err) {
		control_word = control_word_old;
		return err;
	}

	return 0;
}

int can_set_mode_of_operation(int ch, unsigned char node_id, unsigned char opmode)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;

	buf[0] = opmode;
	buf_len = 1;
	err = can_sdo_download(ch, node_id, OD_MODE_OF_OPERATION, 0, buf, buf_len);
	if (err) return err;

	return 0;
}

int can_pdo_download(int ch, unsigned char node_id, unsigned char pdo_id, unsigned char* data, unsigned char len)
{
	assert(ch >= 0 && ch < MAX_BUS);

	int ret;
	long tx_id = 
		(pdo_id == 1 ? COB_ID(COBTYPE_RxPDO1, node_id) :
		(pdo_id == 2 ? COB_ID(COBTYPE_RxPDO2, node_id) :
		(pdo_id == 3 ? COB_ID(COBTYPE_RxPDO3, node_id) :
		(pdo_id == 4 ? COB_ID(COBTYPE_RxPDO4, node_id) : 0))));
	if (!tx_id) return -1;

	ret = canSendMsg(ch, tx_id, len, data, true);
	
	return 0;
}

int can_pdo_set_target_position(int ch, unsigned char node_id, int target_position, unsigned short& control_word)
{
	int err;
	unsigned char data[8];
	unsigned char len = 8;
	unsigned short control_word_old = control_word;
	control_word &= 0xFF8F; // masking irrelevant bits
	control_word |= 0x50; // set new point, target position is relative

	data[0] = LOBYTE(LOWORD(target_position));
	data[1] = HIBYTE(LOWORD(target_position));
	data[2] = LOBYTE(HIWORD(target_position));
	data[3] = HIBYTE(HIWORD(target_position));
	data[4] = 0x01;
	data[5] = LOBYTE(control_word);
	data[6] = HIBYTE(control_word);
	len = 7;
	err = can_pdo_download(ch, node_id, 1, data, len);
	control_word = control_word_old;
	if (err) return err;

	return 0;
}

int can_pdo_rx1(int ch, unsigned char node_id, int target_position, unsigned char mode_of_operation, unsigned short& control_word)
{
	int err;
	unsigned char data[8];
	unsigned char len = 8;

	data[0] = LOBYTE(LOWORD(target_position));
	data[1] = HIBYTE(LOWORD(target_position));
	data[2] = LOBYTE(HIWORD(target_position));
	data[3] = HIBYTE(HIWORD(target_position));
	data[4] = mode_of_operation;
	data[5] = LOBYTE(control_word);
	data[6] = HIBYTE(control_word);
	len = 7;
	err = can_pdo_download(ch, node_id, 1, data, len);
	if (err) return err;

	return 0;
}

int can_set_target_position(int ch, unsigned char node_id, int target_position, unsigned short& control_word)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	unsigned short control_word_old;

	buf[0] = LOBYTE(LOWORD(target_position));
	buf[1] = HIBYTE(LOWORD(target_position));
	buf[2] = LOBYTE(HIWORD(target_position));
	buf[3] = HIBYTE(HIWORD(target_position));
	buf_len = 4;
	err = can_sdo_download(ch, node_id, OD_PROFILED_TARGET_POSITION, 0, buf, buf_len);
	if (err) return err;

	control_word_old = control_word;
	control_word &= 0xFF8F; // masking irrelevant bits
	control_word |= 0x50; // set new point, target position is relative
	buf[0] = LOBYTE(control_word);
	buf[1] = HIBYTE(control_word);
	buf_len = 2;
	err = can_sdo_download(ch, node_id, OD_CONTROLWORD, 0, buf, buf_len);
	control_word = control_word_old;
	if (err) return err;

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

	fn_code = COB_FN_CODE(Rxid);
	node_id = COB_NODE_ID(Rxid);
	return 0;
}

int can_dump_slave(int ch, unsigned char node_id)
{
	return 0;
}

int can_dump_motion_profile(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_upload(ch, node_id, OD_PROFILED_TARGET_POSITION, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tTarget position = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_POSITION_RANGE_LIMIT, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tMin position range limit = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_POSITION_RANGE_LIMIT, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tMax position range limit = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_SOFTWARE_POSITION_LIMIT, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tSoftware min position range limit = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_SOFTWARE_POSITION_LIMIT, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tSoftware max position range limit = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_MAX_PROFILE_VELOCITY, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tMax profile velocity = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_PROFILE_ACCELERATION, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tProfile acceleration = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_PROFILE_DECCELERATION, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tProfile deceleration = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_QUICKSTOP_DECCELERATION, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tQuick stop deceleration = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_MOTION_PROFILE_TYPE, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tMotion profile type = %d\n", MAKEWORD(buf[0], buf[1]));
	}
#endif

	return 0;
}

int can_dump_factors(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_upload(ch, node_id, OD_POLARITY, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tPosition polarity = %d\n", (buf[0]&0x80 ? -1 : 1));
		printf("\tVelocity polarity = %d\n", (buf[0]&0x40 ? -1 : 1));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_POISTION_NOTATION_INDEX, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tPosition notation index = %d\n", buf[0]);
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_POISTION_DIMENSION_INDEX, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tPosition dimension index = %u\n", buf[0]);
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_NOTATION_INDEX, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tVelocity notation index = %d\n", buf[0]);
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_DIMENSION_INDEX, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tVelocity dimension index = %u\n", buf[0]);
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_ACCELERATION_NOTATION_INDEX, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tAcceleration notation index = %d\n", buf[0]);
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_ACCELERATION_DIMENSION_INDEX, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tAcceleration dimension index = %u\n", buf[0]);
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_POSITION_ENCODER_RESOLUTION, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tEncoder increments = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif
	err = can_sdo_upload(ch, node_id, OD_POSITION_ENCODER_RESOLUTION, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tMotor revolutions = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_ENCODER_RESOLUTION, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tEncoder increments per second = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_ENCODER_RESOLUTION, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tMotor revolutions per second = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_POSITION_FACTOR, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tPosition factor (Numerator) = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_POSITION_FACTOR, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tPosition factor (Divisor) = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_ENCODER_FACTOR, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tVelocity encoder factor (Numerator) = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_ENCODER_FACTOR, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tVelocity encoder factor (Divisor) = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_FACTOR_1, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tVelocity factor 1 (Numerator) = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_FACTOR_1, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tVelocity factor 1 (Divisor) = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_FACTOR_2, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tVelocity factor 2 (Numerator) = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_VELOCITY_FACTOR_2, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tVelocity factor 2 (Divisor) = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_ACCELERATION_FACTOR, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tAcceleration factor (Numerator) = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_ACCELERATION_FACTOR, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tAcceleration factor (Divisor) = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	return 0;
}

int can_dump_homing_params(int ch, unsigned char node_id)
{
	int err;
	unsigned char buf[256];
	unsigned short buf_len = 256;
	
	err = can_sdo_upload(ch, node_id, OD_HOMING_OFFSET, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tHoming offset = %d\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_HOMING_METHOD, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tHoming method = %d\n", buf[0]);
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_HOMING_SPEED, 1, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tHoming speed during search for switch = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_HOMING_SPEED, 2, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tHoming speed during search for zero = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	err = can_sdo_upload(ch, node_id, OD_HOMING_ACCELERATION, 0, buf, buf_len);
#ifdef CAN_PRINT_SDO_RESPONSE
	if (!err) {
		printf("\tHoming acceleration = %u\n", MAKELONG(MAKEWORD(buf[0], buf[1]), MAKEWORD(buf[2], buf[3])));
	}
#endif

	return 0;
}

int can_flush(int ch, unsigned char node_id)
{
	unsigned long rx_id;
	unsigned char rx_data[8];
	unsigned char rx_len;
	int err;

	do {
		err = canReadMsg(ch, rx_id, rx_len, rx_data, false);
		if (PCAN_ERROR_QRCVEMPTY == err)
			return 0; // flush returns normally(no remaining messages)
		else if (err)
			return err; // flush returns with error
	} while (true);

	return -1; // should not reach here!
}


CANAPI_END
