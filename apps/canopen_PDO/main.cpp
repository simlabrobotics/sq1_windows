/* canopen_PDO: demonstrate CANopen PDO transmission.
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
#include "canopenAPI.h"
#include "sq1_def.h"
#include "sq1_mem.h"
#include "sq1_PDO.h"

USING_NAMESPACE_SQ1

/////////////////////////////////////////////////////////////////////////////////////////
// for CAN communication
const double delT = 0.005;
int CAN_Ch = 0;
unsigned char NODE_ID = 0x09;
int recvNum = 0;
int sendNum = 0;
double statTime = -1.0;
sQ1_RobotMemory_t vars;

/////////////////////////////////////////////////////////////////////////////////////////
// functions declarations
void PrintInstruction();
void MainLoop();
bool OpenCAN();
void CloseCAN();
void ProcessCANMessage();
#ifdef PeakCAN
extern "C" int getPCANChannelIndex(const char* cname);
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// CAN message dispatcher
void ProcessCANMessage()
{
	unsigned char fn_code;
	unsigned char node_id;
	//unsigned short obj_index;
	//unsigned char sub_index;
	unsigned char len;
	unsigned char data[8];
	unsigned char data_return = 0;
	//int i;

	while (0 == can_get_message(CAN_Ch, fn_code, node_id, len, data, false))
	{
		switch (fn_code)
		{
		case COBTYPE_TxSDO:
			{
			}
			break;

		case COBTYPE_TxPDO1:
			{
				printf("\tTxPDO1 \n");
			}
			break;
		case COBTYPE_TxPDO2:
			{
				printf("\tTxPDO2 \n");
			}
			break;
		case COBTYPE_TxPDO3:
			{
				printf("\tTxPDO3 \n");
			}
			break;
		case COBTYPE_TxPDO4:
			{
				printf("\tTxPDO4 \n");
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Application main-loop. It handles the keyboard events
void MainLoop()
{
	bool bRun = true;
	static int sync_counter = 0;

	while (bRun)
	{
		if (!_kbhit())
		{
			sync_counter++;
			if (sync_counter == 100) {
				can_sync(CAN_Ch);
				sync_counter = 0;
			}
			Sleep(5);
			ProcessCANMessage();
		}
		else
		{
			int c = _getch();
			switch (c)
			{
			case 'q': case 'Q':
				bRun = false;
				break;

			case 's': case 'S':
				can_sync(CAN_Ch);
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
#elif defined(IXXATCAN) || defined(SOFTINGCAN)
	CAN_Ch = 1;
#else
	CAN_Ch = 0;
#endif

	printf(">CAN(%d): open\n", CAN_Ch);
	ret = can_open(CAN_Ch);
	if(ret < 0)
	{
		printf("ERROR command_canopen !!! \n");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Close CAN data channel
void CloseCAN()
{
	int ret;

	printf(">CAN(%d): close\n", CAN_Ch);
	ret = can_close(CAN_Ch);
	if(ret < 0) printf("ERROR command_can_close !!! \n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Print program information and keyboard instructions
void PrintInstruction()
{
	printf("--------------------------------------------------\n");
	printf("CANopen PDO demo: ");

	printf("Keyboard Commands:\n");
	printf("S: Update encoder & DI values\n");
	printf("Q: Quit this program\n");

	printf("--------------------------------------------------\n\n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Program main
int _tmain(int argc, _TCHAR* argv[])
{
	PrintInstruction();

	memset(&vars, 0, sizeof(vars));

	// open CAN channel:
	if (!OpenCAN())
		return -1;

	// reset device:
	printf("reset node...");
	can_nmt_soft_reset(CAN_Ch, NODE_ID);
	Sleep(1000);
	printf("done.\n");

	// PDO mapping:
	printf("PDO mapping...\n");
	can_map_rxpdo1(CAN_Ch, NODE_ID);
	can_map_rxpdo3(CAN_Ch, NODE_ID);
	can_map_txpdo1(CAN_Ch, NODE_ID);
	can_map_txpdo3(CAN_Ch, NODE_ID);

	// set communication mode OPERATIONAL:
	printf("set communication mode OPERATIONAL...\n");
	can_nmt_node_start(CAN_Ch, NODE_ID);

	// loop wait user input:
	printf("main loop...\n");
	MainLoop();

	// set communication mode PREPARED:
	printf("set communication mode STOPPED...\n");
	can_nmt_node_stop(CAN_Ch, NODE_ID);

	// flush can messages:
	printf("flush can messages...\n");
	can_flush(CAN_Ch, NODE_ID);

	// close CAN channel:
	CloseCAN();

	return 0;
}
