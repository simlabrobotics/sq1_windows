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
#include "IXXAT/vcinpl.h"
}
#include "select.hpp"
#include "canDef.h"
#include "canAPI.h"


#define CH_COUNT			((int)4) // number of CAN channels

CANAPI_BEGIN
CANAPI_EXTERN_C_BEGIN

//////////////////////////////////////////////////////////////////////////
// global variables
//////////////////////////////////////////////////////////////////////////
static HANDLE hDevice[CH_COUNT] = {(HANDLE) 0, (HANDLE) 0, (HANDLE) 0, (HANDLE) 0};  // device handle
static LONG   lCtrlNo[CH_COUNT] = {         0,          0,          0,          0};  // controller number
static HANDLE hCanCtl[CH_COUNT] = {(HANDLE)-1, (HANDLE)-1, (HANDLE)-1, (HANDLE)-1};  // controller handle 
static HANDLE hCanChn[CH_COUNT] = {(HANDLE)-1, (HANDLE)-1, (HANDLE)-1, (HANDLE)-1};  // channel handle

//////////////////////////////////////////////////////////////////////////
// static function prototypes
//////////////////////////////////////////////////////////////////////////
HRESULT SelectDevice ( UINT32 dwCanChNo, BOOL fUserSelect );
HRESULT InitSocket   ( UINT32 dwCanChNo, UINT32 dwCanNo );
void    FinalizeApp  ( UINT32 dwCanChNo );
void    DisplayError ( /*UINT32 dwCanChNo,*/ HRESULT hResult );


//////////////////////////////////////////////////////////////////////////
// Public functions (CAN API)
//////////////////////////////////////////////////////////////////////////
int initCAN(int bus)
{
	assert(bus >= 1 && bus <= CH_COUNT);
	if (bus <= 0) // bus index should be greater than 0
		return -1;

	HRESULT hResult;

	hResult = SelectDevice( bus-1, TRUE );
	if ( VCI_OK != hResult ) {
		DisplayError(hResult);
		return hResult;
	}

	hResult = InitSocket( bus-1, lCtrlNo[bus-1] );
	DisplayError(hResult);
	return hResult;
}

int freeCAN(int bus)
{
	assert(bus >= 1 && bus <= CH_COUNT);
	if (bus <= 0) // bus index should be greater than 0
		return -1;

	FinalizeApp( bus-1 );
	return 0;
}

int canReadMsg(int bus, unsigned long &id, unsigned char &len, unsigned char *data, bool blocking)
{
	assert(bus >= 1 && bus <= CH_COUNT);
	if (bus <= 0) // bus index should be greater than 0
		return -1;

	HRESULT hResult;
	CANMSG  sCanMsg;
	HANDLE  hCAN = hCanChn[bus-1];

	//hResult = canChannelReadMessage(hCAN, (blocking ? INFINITE : 0), &sCanMsg);
	if (blocking)
		hResult = canChannelReadMessage(hCAN, INFINITE, &sCanMsg);
	else
		hResult = canChannelPeekMessage(hCAN, &sCanMsg);
	
	if (hResult == VCI_OK)
	{
		if (sCanMsg.uMsgInfo.Bytes.bType == CAN_MSGTYPE_DATA)
		{
			if (sCanMsg.uMsgInfo.Bits.rtr == 0)
			{
				len = (int)( sCanMsg.uMsgInfo.Bits.dlc );
				for(int nd=0; nd<len; nd++) data[nd] = sCanMsg.abData[nd];

#ifdef _DEBUG
				/*UINT8 j;
				printf("\nTime: %10u  ID: %3X  DLC: %1u  Data:",
					sCanMsg.dwTime,
					sCanMsg.dwMsgId,
					sCanMsg.uMsgInfo.Bits.dlc);
				for (j = 0; j < sCanMsg.uMsgInfo.Bits.dlc; j++)
					printf(" %.2X", sCanMsg.abData[j]);
				printf("\n");*/
#endif
			}
			else
			{
				printf("\nTime: %10u ID: %3X  DLC: %1u  Remote Frame",
						sCanMsg.dwTime,
						sCanMsg.dwMsgId,
						sCanMsg.uMsgInfo.Bits.dlc);
			}
		}
		else if (sCanMsg.uMsgInfo.Bytes.bType == CAN_MSGTYPE_INFO)
		{
			//
			// show informational frames
			//
			switch (sCanMsg.abData[0])
			{
				case CAN_INFO_START: printf("\nCAN started..."); break;
				case CAN_INFO_STOP : printf("\nCAN stoped...");  break;
				case CAN_INFO_RESET: printf("\nCAN reseted..."); break;
			}
		}
		else if (sCanMsg.uMsgInfo.Bytes.bType == CAN_MSGTYPE_ERROR)
		{
			//
			// show error frames
			//
			switch (sCanMsg.abData[0])
			{
				case CAN_ERROR_STUFF: printf("\nstuff error...");          break; 
				case CAN_ERROR_FORM : printf("\nform error...");           break; 
				case CAN_ERROR_ACK  : printf("\nacknowledgment error..."); break;
				case CAN_ERROR_BIT  : printf("\nbit error...");            break; 
				case CAN_ERROR_CRC  : printf("\nCRC error...");            break; 
				case CAN_ERROR_OTHER:
				default             : printf("\nother error...");          break;
			}
		}
    }
	else
	{
		 if (VCI_E_RXQUEUE_EMPTY != hResult &&
			 VCI_E_TIMEOUT != hResult)
			 DisplayError(hResult);
	}
	
	return hResult;
}

int canSendMsg(int bus, unsigned long id, unsigned char len, unsigned char *data, bool blocking)
{
	assert(bus >= 1 && bus <= CH_COUNT);
	if (bus <= 0) // bus index should be greater than 0
		return -1;

	HRESULT hResult;
	CANMSG  sCanMsg;
	UINT8   i;
	HANDLE  hCAN = hCanChn[bus-1];
	UINT8   mode = STD;

	sCanMsg.dwTime   = 0;
	sCanMsg.dwMsgId  = id;    // CAN message identifier

	sCanMsg.uMsgInfo.Bytes.bType  = CAN_MSGTYPE_DATA;
	sCanMsg.uMsgInfo.Bytes.bFlags = CAN_MAKE_MSGFLAGS(len,0,0,0,mode);
	sCanMsg.uMsgInfo.Bits.srr     = 0;

	for (i = 0; i < sCanMsg.uMsgInfo.Bits.dlc; i++)
	{
		sCanMsg.abData[i] = ((unsigned char*)data)[i];
	}

	// write the CAN message into the transmit FIFO
	hResult = canChannelSendMessage(hCAN, INFINITE, &sCanMsg);

	if (hResult != VCI_OK)
	{
		DisplayError(hResult);
	}

	return hResult;
}





/**
  Selects the first CAN adapter.

  @param fUserSelect
    If this parameter is set to TRUE the functions display a dialog box which 
    allows the user to select the device.

  @return
    VCI_OK on success, otherwise an Error code
*/
HRESULT SelectDevice( UINT32 dwCanChNo, BOOL fUserSelect )
{
  HRESULT hResult; // error code

  if (fUserSelect == FALSE)
  {
    HANDLE        hEnum;   // enumerator handle
    VCIDEVICEINFO sInfo;   // device info

    //
    // open the device list
    //
    hResult = vciEnumDeviceOpen(&hEnum);

    //
    // retrieve information about the first
    // device within the device list
    //
    if (hResult == VCI_OK)
    {
      hResult = vciEnumDeviceNext(hEnum, &sInfo);
    }

    //
    // close the device list (no longer needed)
    //
    vciEnumDeviceClose(hEnum);

    //
    // open the device
    //
    if (hResult == VCI_OK)
    {
      hResult = vciDeviceOpen(sInfo.VciObjectId, &(hDevice[dwCanChNo]));
    }

    //
    // always select controller 0
    //
    lCtrlNo[dwCanChNo] = 0;
  }
  else
  {
    //
    // open a device selected by the user
    //
    hResult = SocketSelectDlg(NULL, VCI_BUS_CAN, &(hDevice[dwCanChNo]), &(lCtrlNo[dwCanChNo]));
  }

  DisplayError(hResult);
  return hResult;
}

/**
  Opens the specified socket, creates a message channel, initializes
  and starts the CAN controller.

  @param dwCanNo
    Number of the CAN controller to open.

  @return
    VCI_OK on success, otherwise an Error code

  @note
    If <dwCanNo> is set to 0xFFFFFFFF, the function shows a dialog box
    which allows the user to select the VCI device and CAN controller.
*/
HRESULT InitSocket( UINT32 dwCanChNo, UINT32 dwCanNo )
{
  HRESULT hResult;

  //
  // create a message channel
  //
  if (hDevice[dwCanChNo] != NULL)
  {
    //
    // create and initialize a message channel
    //
    hResult = canChannelOpen(hDevice[dwCanChNo], dwCanNo, FALSE, &(hCanChn[dwCanChNo]));

    //
    // initialize the message channel
    //
    if (hResult == VCI_OK)
    {
      UINT16 wRxFifoSize  = 1024;
      UINT16 wRxThreshold = 1;
      UINT16 wTxFifoSize  = 128;
      UINT16 wTxThreshold = 1;

      hResult = canChannelInitialize( hCanChn[dwCanChNo],
                                      wRxFifoSize, wRxThreshold,
                                      wTxFifoSize, wTxThreshold);
    }

    //
    // activate the CAN channel
    //
    if (hResult == VCI_OK)
    {
      hResult = canChannelActivate(hCanChn[dwCanChNo], TRUE);
    }

    //
    // open the CAN controller
    //
    if (hResult == VCI_OK)
    {
      hResult = canControlOpen(hDevice[dwCanChNo], dwCanNo, &(hCanCtl[dwCanChNo]));
      // this function fails if the controller is in use
      // by another application.
    }

    //
    // initialize the CAN controller
    //
    if (hResult == VCI_OK)
    { 
      hResult = canControlInitialize(hCanCtl[dwCanChNo], CAN_OPMODE_STANDARD | CAN_OPMODE_ERRFRAME,
		                             CAN_BT0_1000KB, CAN_BT1_1000KB);
    }

    //
    // set the acceptance filter
    //
    if (hResult == VCI_OK)
    { 
       hResult = canControlSetAccFilter( hCanCtl[dwCanChNo], FALSE,
                                         CAN_ACC_CODE_ALL, CAN_ACC_MASK_ALL);
    }

    //
    // start the CAN controller
    //
    if (hResult == VCI_OK)
    {
      hResult = canControlStart(hCanCtl[dwCanChNo], TRUE);
    }
  }
  else
  {
    hResult = VCI_E_INVHANDLE;
  }

  DisplayError(hResult);
  return hResult;
}

/**
  Finalizes the application
*/
void FinalizeApp( UINT32 dwCanChNo )
{
  //
  // close all open handles

  canControlReset(hCanCtl[dwCanChNo]);
  canChannelClose(hCanChn[dwCanChNo]);
  canControlClose(hCanCtl[dwCanChNo]);

  vciDeviceClose(hDevice[dwCanChNo]);

  hCanCtl[dwCanChNo] = (HANDLE) 0;
  hCanChn[dwCanChNo] = (HANDLE)-1;
  hDevice[dwCanChNo] = (HANDLE)-1;
}

/**
  This function displays a message box for the specified error code.

  @param hResult
    Error code or -1 to display the error code returned by GetLastError().
*/
void DisplayError( /*UINT32 dwCanChNo,*/ HRESULT hResult )
{
  char szError[VCI_MAX_ERRSTRLEN];

  if (hResult != NO_ERROR)
  {
    if (hResult == -1)
      hResult = GetLastError();

    szError[0] = 0;
    vciFormatError(hResult, szError, sizeof(szError));
    //MessageBoxA(NULL, szError, "rDeviceAllegroHandIXXATCAN", MB_OK | MB_ICONSTOP);
    printf("%s\n", szError);
  }
}

CANAPI_EXTERN_C_END
CANAPI_END
