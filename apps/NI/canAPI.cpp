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
#endif
#include <windows.h>
#include <malloc.h>
#include <assert.h>
//project headers
extern "C" {
#include "NI/nican.h"
}
#include "canDef.h"
#include "canAPI.h"

CANAPI_BEGIN
CANAPI_EXTERN_C_BEGIN

#define CH_COUNT			(int)4 // number of CAN channels


NCTYPE_OBJH hd[CH_COUNT] = {0, 0, 0, 0};

#define canMSG_MASK             0x00ff      // Used to mask the non-info bits
#define canMSG_RTR              0x0001      // Message is a remote request
#define canMSG_STD              0x0002      // Message has a standard ID
#define canMSG_EXT              0x0004      // Message has an extended ID
#define canMSG_WAKEUP           0x0008      // Message to be sent / was received in wakeup mode
#define canMSG_NERR             0x0010      // NERR was active during the message
#define canMSG_ERROR_FRAME      0x0020      // Message is an error frame
#define canMSG_TXACK            0x0040      // Message is a TX ACK (msg is really sent)
#define canMSG_TXRQ             0x0080      // Message is a TX REQUEST (msg is transfered to the chip)


/* This function converts the absolute time obtained from ncReadMult into a
   string. */
void AbsTimeToString(NCTYPE_ABS_TIME *time, char *TimeString, int len)
{

   SYSTEMTIME	stime;
   FILETIME		localftime;

   FileTimeToLocalFileTime((FILETIME *)(time), &localftime);
   FileTimeToSystemTime(&localftime, &stime);
#ifdef WIN32
   sprintf_s(TimeString, len, "%02d:%02d:%02d.%03d",
            stime.wHour, stime.wMinute, stime.wSecond, 
            stime.wMilliseconds);
#else
   sprintf(TimeString, "%02d:%02d:%02d.%03d",
            stime.wHour, stime.wMinute, stime.wSecond, 
            stime.wMilliseconds);
#endif
}

/* Print a description of an NI-CAN error/warning. */
void PrintStat(NCTYPE_OBJH& CANHandle, NCTYPE_STATUS Status, char *source) 
{
	char StatusString[1024];
     
	if (Status != 0) 
	{
		ncStatusToString(Status, sizeof(StatusString), StatusString);
		printf("\n%s\nSource = %s\n", StatusString, source);

		// On error, close object handle.
		printf("<< CAN: Close\n");
		ncCloseObject(CANHandle);
		CANHandle = 0;
		//exit(1);
	}
}

/* Print read frame */
void PrintRxFrame(NCTYPE_OBJH& CANHandle, NCTYPE_CAN_STRUCT& RxFrame)
{
	char output[15];
	char CharBuff[50];
	AbsTimeToString(&RxFrame.Timestamp, &output[0], 15);
	printf("%s     ", output);
#ifdef WIN32
	sprintf_s(&CharBuff[0], 50, "%8.8X", RxFrame.ArbitrationId);
#else
	sprintf (&CharBuff[0], "%8.8X", RxFrame.ArbitrationId);
#endif
	printf("%s     ", CharBuff);
#ifdef WIN32
	sprintf_s(&CharBuff[0], 50, "%s","CAN Data Frame");
#else
	sprintf (&CharBuff[0], "%s","CAN Data Frame");
#endif
	printf("%s     ", CharBuff); 
#ifdef WIN32
	sprintf_s(&CharBuff[0], 50, "%1d", RxFrame.DataLength);
#else
	sprintf (&CharBuff[0], "%1d", RxFrame.DataLength);
#endif
	printf("%s     ", CharBuff); 
	for (int j=0; j<RxFrame.DataLength; j++)
	{
#ifdef WIN32
		sprintf_s(CharBuff, 50, " %02X", RxFrame.Data[j]);
#else
		sprintf(CharBuff, " %02X", RxFrame.Data[j]);
#endif
		printf("%s", CharBuff); 
	}
	printf("\n");
}


/*========================================*/
/*       Public functions (CAN API)       */
/*========================================*/
int initCAN(int bus)
{
	NCTYPE_ATTRID		AttrIdList[8];
	NCTYPE_UINT32		AttrValueList[8];
	//NCTYPE_UINT32		Baudrate = 125000;  // BAUD_125K
	NCTYPE_UINT32		Baudrate = NC_BAUD_1000K;
	char				Interface[15];
	NCTYPE_STATUS		Status;
	NCTYPE_OBJH			TxHandle;
	
	sprintf_s(Interface, "CAN%d", bus);
	
	// Configure the CAN Network Interface Object	
	AttrIdList[0] =     NC_ATTR_BAUD_RATE;   
	AttrValueList[0] =  Baudrate;
	AttrIdList[1] =     NC_ATTR_START_ON_OPEN;
	AttrValueList[1] =  NC_TRUE;
	AttrIdList[2] =     NC_ATTR_READ_Q_LEN;
	AttrValueList[2] =  100;
	AttrIdList[3] =     NC_ATTR_WRITE_Q_LEN;
	AttrValueList[3] =  10;	
	AttrIdList[4] =     NC_ATTR_CAN_COMP_STD;
	AttrValueList[4] =  0;//0xCFFFFFFF;
	AttrIdList[5] =     NC_ATTR_CAN_MASK_STD;
	AttrValueList[5] =  NC_CAN_MASK_STD_DONTCARE;
	AttrIdList[6] =     NC_ATTR_CAN_COMP_XTD;
	AttrValueList[6] =  0;//0xCFFFFFFF;
	AttrIdList[7] =     NC_ATTR_CAN_MASK_XTD;
	AttrValueList[7] =  NC_CAN_MASK_XTD_DONTCARE;

	
	printf("<< CAN: Config\n");
	Status = ncConfig(Interface, 2, AttrIdList, AttrValueList);
	if (Status < 0) 
	{
		PrintStat(hd[bus], Status, "ncConfig");
		return Status;
	}
	printf("   - Done\n");
    
	// open the CAN Network Interface Object
	printf("<< CAN: Open Channel\n");
	Status = ncOpenObject (Interface, &TxHandle);
	if (Status < 0) 
	{
		PrintStat(hd[bus], Status, "ncOpenObject");
		return Status;
	}
	hd[bus] = TxHandle;
	printf("   - Done\n");
	return 0; // CanSuccess
}

int resetCAN(int bus)
{
	NCTYPE_STATUS		Status;
	//int ret_c;
	//long id;
	//unsigned char sdata[8], rdata[8];
	//unsigned int dlc, flags;
	//unsigned long timestamp;

	printf("<< CAN: Reset Bus\n");

	// stop the CAN Network
	Status = ncAction(hd[bus], NC_OP_STOP, 0);
	if (Status < 0) 
	{
		PrintStat(hd[bus], Status, "ncAction(NC_OP_STOP)");
		return Status;
	}
	// start the CAN Network
	Status = ncAction(hd[bus], NC_OP_START, 0);
	if (Status < 0) 
	{
		PrintStat(hd[bus], Status, "ncAction(NC_OP_START)");
		return Status;
	}

	/*while (true)
	{
		memset(rdata, NULL, sizeof(rdata));
		Status = canRead(TxHandle, &id, rdata, &dlc, &flags, &timestamp);
		if (Status != 0) break;
		printf("    %ld+%ld (%d)", id-id%128, id%128, dlc);
		for(int nd=0; nd<(int)dlc; nd++) printf(" %3d ", rdata[nd]);
		printf("\n");
	}*/

	/*int nc;

	printf("<< CAN: Reset Bus\n");
	for(nc=0; nc<CH_COUNT; nc++) {
		ret_c = canResetBus(hd[nc]);
		if (ret_c < 0) return ret_c;
		printf("    Ch.%2d (OK)\n", nc, ret_c);
	}*/
	printf("   - Done\n");
	Sleep(200);
	return 0; // CanSuccess
}

int freeCAN(int bus)
{
	NCTYPE_STATUS		Status;

	if (!hd[bus])
		return -1;

	printf("<< CAN: Close\n");
	Status = ncCloseObject(hd[bus]);    
	if (Status < 0)
	{
		PrintStat(hd[bus], Status, "ncCloseObject");
		return Status;
	}
	hd[bus] = 0;
	printf("   - Done\n");
	return 0; // CanSuccess
}

int canSendMsg(int bus, unsigned long id, unsigned char len, unsigned char *data, bool /*blocking*/) 
{
	NCTYPE_STATUS		Status;
	NCTYPE_CAN_FRAME	TxFrame;

	if (!hd[bus])
		return -1;

	TxFrame.IsRemote = NC_FRMTYPE_DATA;
	TxFrame.ArbitrationId = id;
	TxFrame.DataLength = len;
	for (int i=0; i<len; i++)
		TxFrame.Data[i] = ((NCTYPE_UINT8_P)data)[i];
	Status = ncWrite(hd[bus], sizeof(NCTYPE_CAN_FRAME), &TxFrame);
	if (Status < 0)
	{
		PrintStat(hd[bus], Status, "ncWrite");
		return Status;
	}
	return Status;
}

int canReadMsg(int bus, unsigned long& id, unsigned char& len, unsigned char *data, bool blocking) 
{
	NCTYPE_STATUS		Status;
	NCTYPE_CAN_STRUCT	RxFrame;

	if (!hd[bus])
		return -1;
	
	memset(&RxFrame, 0, sizeof(NCTYPE_CAN_FRAME));
	//RxFrame.FrameType = NC_FRMTYPE_DATA;
	RxFrame.DataLength = len;
	Status = ncRead(hd[bus], sizeof(NCTYPE_CAN_STRUCT), &RxFrame);
	if (Status < 0 || NC_FRMTYPE_DATA != RxFrame.FrameType)
	{
		PrintStat(hd[bus], Status, "ncRead");
		return Status;
	}
	else if (Status == CanWarnOldData)
	{
		len = 0;
		return Status;
	}

	id = RxFrame.ArbitrationId;
	//(*time) = (FILETIME)(RxFrame.Timestamp);
	len = RxFrame.DataLength;
	for (int i=0; i<RxFrame.DataLength; i++)
		((NCTYPE_UINT8_P)data)[i] = RxFrame.Data[i];
#ifdef CAN_PRINT_Rx_MESSAGE
	PrintRxFrame(hd[bus], RxFrame);
#endif
	return Status;
}

int canReadMsgWait(int bus, unsigned long& id, unsigned char& len, unsigned char *data, bool blocking) 
{
	NCTYPE_STATUS		Status;
	NCTYPE_CAN_STRUCT	RxFrame;
	NCTYPE_STATE		currentState;
	unsigned long		timeout;

	if (!hd[bus])
		return -1;

	timeout = 1000;
	Status = ncWaitForState(hd[bus], NC_ST_READ_AVAIL, timeout, &currentState);
	if (Status < 0)
	{
		PrintStat(hd[bus], Status, "ncWaitForState");
		return Status;
	}

	memset(&RxFrame, 0, sizeof(NCTYPE_CAN_FRAME));
	RxFrame.FrameType = NC_FRMTYPE_DATA;
	RxFrame.DataLength = len;
	Status = ncRead(hd[bus], sizeof(NCTYPE_CAN_STRUCT), &RxFrame);
	if (Status < 0)
	{
		PrintStat(hd[bus], Status, "ncRead");
		return Status;
	}

	id = RxFrame.ArbitrationId;
	//(*time) = (FILETIME)(RxFrame.Timestamp);
	len = RxFrame.DataLength;
	for (int i=0; i<RxFrame.DataLength; i++)
		((NCTYPE_UINT8_P)data)[i] = RxFrame.Data[i];
#ifdef CAN_PRINT_Rx_MESSAGE
	PrintRxFrame(hd[bus], RxFrame);
#endif
	return Status;
}


CANAPI_EXTERN_C_END
CANAPI_END
