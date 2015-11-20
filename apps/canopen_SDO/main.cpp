/* enum_devices: enumerate all canopen slave nodes using LSS(Layer Setting Services).
 *
 * Copyright (c) 2016 SimLab Co., Ltd. http://www.simlab.co.kr/
 * 
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose, 
 * including commercial applications, and to alter it and redistribute it freely. 
 */

#include "windows.h"
#include <conio.h>
#include <process.h>
#include <tchar.h>
#include <stdio.h>
#include "canAPI.h"


/////////////////////////////////////////////////////////////////////////////////////////
// for CAN communication
const double delT = 0.005;
int CAN_Ch = 0;
unsigned char NODE_ID = 0x01;
bool ioThreadRun = false;
uintptr_t ioThread = 0;
int recvNum = 0;
int sendNum = 0;
double statTime = -1.0;

/////////////////////////////////////////////////////////////////////////////////////////
// functions declarations
void PrintInstruction();
void MainLoop();
bool OpenCAN();
void CloseCAN();
extern int getPCANChannelIndex(const char* cname);

/////////////////////////////////////////////////////////////////////////////////////////
// CAN communication thread
static unsigned int __stdcall ioThreadProc(void* inst)
{
	unsigned char fn_type;
	unsigned char node_id;
	unsigned short obj_id;
	int len;
	unsigned char data[8];
	unsigned char data_return = 0;
	int i;

	while (ioThreadRun)
	{
		while (0 == can_get_message(CAN_Ch, &fn_type, &node_id, &len, data, FALSE))
		{
			switch (fn_type)
			{
			case COBTYPE_TxSDO:
				{
					unsigned char scs			= ((data[0] & 0xe0) >> 5);
					unsigned char num_of_bytes	= ((data[0] & 0x0c) >> 2);
					unsigned char tx_type		= ((data[0] & 0x02) >> 1);
					unsigned char size_indicator= ((data[0] & 0x01) >> 0);
					printf("\tscs=%d, num_of_bytes=%d, tx_type=%d, size_indicator=%d\n", scs, num_of_bytes, tx_type, size_indicator);

					obj_id = MAKEWORD(data[1], data[2]);
					switch (obj_id)
					{
					case OD_CANCTL_TYPE:
						{
							printf("\tdevice profile number = %d\n", MAKEWORD(data[6], data[7]));
							printf("\tnumber of SDOs supported = %d\n", MAKEWORD(data[4], data[5]));
						}
						break;
					case OD_MANUFACTURER_DEVICE_NAME:
						{
							printf("\tdevice name = %c%c%c%c\n", data[4], data[5], data[6], data[7]);
						}
						break;
					case OD_HW_VERSION:
						{
							printf("\thw version = %c%c%c%c\n", data[4], data[5], data[6], data[7]);
						}
						break;
					case OD_SW_VERSION:
						{
							printf("\tsw version = %c%c%c%c\n", data[4], data[5], data[6], data[7]);
						}
						break;
					case OD_NODEID:
						{
							printf("\tnode id = %d\n", data[4]);
						}
						break;
					case OD_LSS_ADDRESS:
						{
							switch (data[3])
							{
							case 1: printf("\tvendor id = %d\n", MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))); break;
							case 2: printf("\tproduct id = %d\n", MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))); break;
							case 3: printf("\trevision number = %d\n", MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))); break;
							case 4: printf("\tserial number = %d\n", MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))); break;
							}
						}
						break;
					}
				}
				break;
			case COBTYPE_TxPDO1:
				{
				}
				break;
			}
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Application main-loop. It handles the keyboard events
void MainLoop()
{
	bool bRun = true;
	int i;

	while (bRun)
	{
		if (!_kbhit())
		{
			Sleep(5);
		}
		else
		{
			int c = _getch();
			switch (c)
			{
			case 'q':
				bRun = false;
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Open a CAN data channel
bool OpenCAN()
{
	int ret;
	
#if defined(PeakCAN)
	CAN_Ch = getPCANChannelIndex("PCAN_PCIBUS1");
#elif defined(IXXATCAN)
	CAN_Ch = 1;
#elif defined(SOFTINGCAN)
	CAN_Ch = 1;
#elif defined(NICAN)
	CAN_Ch = 0;
#else
	CAN_Ch = 1;
#endif

	printf(">CAN(%d): open\n", CAN_Ch);
	ret = can_open(CAN_Ch);
	if(ret < 0)
	{
		printf("ERROR command_canopen !!! \n");
		return false;
	}

	recvNum = 0;
	sendNum = 0;
	statTime = 0.0;

	ioThreadRun = true;
	ioThread = _beginthreadex(NULL, 0, ioThreadProc, NULL, 0, NULL);
	printf(">CAN: starts listening CAN frames\n");
	
	//printf(">CAN: system init\n");
	//ret = can_sys_init(CAN_Ch, NODE_ID, 5/*msec*/);
	//if(ret < 0)
	//{
	//	printf("ERROR command_can_sys_init !!! \n");
	//	can_close(CAN_Ch);
	//	return false;
	//}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Close CAN data channel
void CloseCAN()
{
	int ret;

	if (ioThreadRun)
	{
		printf(">CAN: stoped listening CAN frames\n");
		ioThreadRun = false;
		WaitForSingleObject((HANDLE)ioThread, INFINITE);
		CloseHandle((HANDLE)ioThread);
		ioThread = 0;
	}

	printf(">CAN(%d): close\n", CAN_Ch);
	ret = can_close(CAN_Ch);
	if(ret < 0) printf("ERROR command_can_close !!! \n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Print program information and keyboard instructions
void PrintInstruction()
{
	printf("--------------------------------------------------\n");
	printf("enum devices: \n\n");

	printf("Keyboard Commands:\n");
	printf("Q: Quit this program\n");
	printf("--------------------------------------------------\n\n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Program main
int _tmain(int argc, _TCHAR* argv[])
{
	PrintInstruction();

	// open CAN channel:
	if (!OpenCAN())
		return -1;

	// query device type:
	can_query_device_type(CAN_Ch, NODE_ID);
	Sleep(1000);

	// query Manufacturer's device name:
	//can_query_device_name(CAN_Ch, NODE_ID);
	
	// query H/W, S/W version:
	//can_query_hw_version(CAN_Ch, NODE_ID);
	//can_query_sw_version(CAN_Ch, NODE_ID);
	
	// query LSS address:
	can_query_lss_address(CAN_Ch, NODE_ID);

	// loop wait user input:
	MainLoop();

	// close CAN channel:
	CloseCAN();

	return 0;
}
