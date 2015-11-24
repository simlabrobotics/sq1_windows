/* app_SQ1 : Defines the entry point for the console application.
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
#include "sq1_def.h"
#include "sq1_mem.h"
#include "sq1_PDO.h"

USING_NAMESPACE_SQ1

/////////////////////////////////////////////////////////////////////////////////////////
// for CAN communication
const double delT = 0.005;
int CAN_Ch = 0;
unsigned char NODE_ID = 0x09;
bool ioThreadRun = false;
uintptr_t ioThread = 0;
int recvNum = 0;
int sendNum = 0;
double statTime = -1.0;
sQ1_RobotMemory_t vars;
unsigned short controlWord = 0;
unsigned short statusWord = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// functions declarations
void PrintInstruction();
void MainLoop();
bool OpenCAN();
void CloseCAN();
void StartCANListenThread();
void StopCANListenThread();
extern int getPCANChannelIndex(const char* cname);

/////////////////////////////////////////////////////////////////////////////////////////
// motion declarations
void MotionStretch();
void MotionSquat();
void MotionWalkReady();
void MotionWalk();

/////////////////////////////////////////////////////////////////////////////////////////
// CAN communication thread
static unsigned int __stdcall ioThreadProc(void* inst)
{
	unsigned char fn_code;
	unsigned char node_id;
	//unsigned short obj_index;
	//unsigned char sub_index;
	unsigned char len;
	unsigned char data[8];
	unsigned char data_return = 0;
	//int i;

	while (ioThreadRun)
	{
		while (0 == can_get_message(CAN_Ch, fn_code, node_id, len, data, false))
		{
			switch (fn_code)
			{
			case COBTYPE_TxSDO:
				{
				}
				break;

			case COBTYPE_TxPDO1:
			case COBTYPE_TxPDO2:
			case COBTYPE_TxPDO3:
			case COBTYPE_TxPDO4:
				{
					printf("\tTxPDO%d \n", (fn_code-COBTYPE_TxPDO1+1));
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
	static int sync_counter = 0;

	while (bRun)
	{
		if (!_kbhit())
		{
			Sleep(5);
			sync_counter++;
			if (sync_counter == 100) {
				can_sync(CAN_Ch);
				sync_counter = 0;
			}
		}
		else
		{
			int c = _getch();
			switch (c)
			{
			case 'q':
				bRun = false;
				break;

			case 's':
				can_sync(CAN_Ch);
				break;
			
			case '1':
				MotionStretch();
				break;

			case '2':
				MotionSquat();
				break;

			case '3':
				MotionWalkReady();
				break;

			case '4':
				MotionWalk();
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

	//StartCANListenThread();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Close CAN data channel
void CloseCAN()
{
	int ret;

	StopCANListenThread();

	printf(">CAN(%d): close\n", CAN_Ch);
	ret = can_close(CAN_Ch);
	if(ret < 0) printf("ERROR command_can_close !!! \n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Start/Stop CAN message listener
void StartCANListenThread()
{
	recvNum = 0;
	sendNum = 0;
	statTime = 0.0;

	ioThreadRun = true;
	ioThread = _beginthreadex(NULL, 0, ioThreadProc, NULL, 0, NULL);
	printf(">CAN: starts listening CAN frames\n");
}

void StopCANListenThread()
{
	if (ioThreadRun)
	{
		printf(">CAN: stoped listening CAN frames\n");
		ioThreadRun = false;
		WaitForSingleObject((HANDLE)ioThread, INFINITE);
		CloseHandle((HANDLE)ioThread);
		ioThread = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Print program information and keyboard instructions
void PrintInstruction()
{
	printf("--------------------------------------------------\n");
	printf("mySQ1: ");

	printf("Keyboard Commands:\n");
	printf("1: Stretch All Legs Downwards\n");
	printf("2: Squat Motion\n");	
	printf("3: Walk-Ready Position\n");
	printf("4: Start Walk\n");

	printf("S: Update encoder & DI values\n");
	printf("E: E-STOP\n");
	printf("Q: Quit this program\n");

	printf("--------------------------------------------------\n\n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Demo motions:
void MotionStretch()
{
}

void MotionSquat()
{
}

void MotionWalkReady()
{
}

void MotionWalk()
{
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
	can_pdo_map(CAN_Ch, NODE_ID);

	// servo off(make it sure motor drives are in servo-off state):
//	printf("servo off...\n");
//	can_servo_off(CAN_Ch, NODE_ID, controlWord);

	// set mode of operation:
	printf("set mode of operation...\n");
	can_set_mode_of_operation(CAN_Ch, NODE_ID, OP_MODE_PROFILED_POSITION);
	printf("query mode of operation...\n");
	can_query_mode_of_operation_display(CAN_Ch, NODE_ID);

	// servo on:
	printf("servo on...\n");
	can_servo_on(CAN_Ch, NODE_ID, controlWord);

	// set communication mode OPERATIONAL:
	printf("set communication mode OPERATIONAL...\n");
	can_nmt_node_start(CAN_Ch, NODE_ID);

	// start periodic communication:
	printf("start periodic communication...\n");
	StartCANListenThread();

	// loop wait user input:
	printf("main loop...\n");
	MainLoop();

	// stop periodic communication:
	printf("stop periodic communication...\n");
	StopCANListenThread();
	
	// set communication mode PREPARED:
	printf("set communication mode STOPPED...\n");
	can_nmt_node_stop(CAN_Ch, NODE_ID);

	// flush can messages:
	printf("flush can messages...\n");
	can_flush(CAN_Ch, NODE_ID);

	// servo off:
	printf("servo off...\n");
	can_servo_off(CAN_Ch, NODE_ID, controlWord);

	// close CAN channel:
	CloseCAN();

	return 0;
}
