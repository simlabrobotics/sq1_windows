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
#include "canopenAPI.h"
#include "sq1_def.h"
#include "sq1_mem.h"
#include "sq1_PDO.h"

USING_NAMESPACE_SQ1

/////////////////////////////////////////////////////////////////////////////////////////
// for CAN communication
const double delT = 0.005;
const unsigned int CAN_Ch_COUNT = LEG_COUNT;
const unsigned int NODE_COUNT = LEG_JDOF;
int CAN_Ch[CAN_Ch_COUNT] = {0, 0, 0, 0};
const bool CAN_Ch_Enabled[CAN_Ch_COUNT] = {false, false, true, false};
const bool NODE_Enabled[LEG_COUNT][LEG_JDOF] = {
	{false, false, false},
	{false, false, false},
	{false, false, true},
	{false, false, false}
};
bool ioThreadRun[CAN_Ch_COUNT] = {false, false, false, false};
uintptr_t ioThread[CAN_Ch_COUNT] = {0, 0, 0, 0};
int recvNum[CAN_Ch_COUNT] = {0, 0, 0, 0};
int sendNum[CAN_Ch_COUNT] = {0, 0, 0, 0};
double statTime[CAN_Ch_COUNT] = {-1.0, -1.0, -1.0, -1.0};
sQ1_RobotMemory_t vars;
long targetPosition = 0;
unsigned long targetVelocity = 0;
unsigned char modeOfOperation = OP_MODE_PROFILED_POSITION;
unsigned short controlWord = 0;
unsigned short statusWord = 0;

static void printBinary(unsigned char by)
{
	char sz[9];
	int i;
	sz[0] = '\0';
	for (i=128; i>0; i>>=1)
		printf("%c", (by&i?'1':'0'));
}

/////////////////////////////////////////////////////////////////////////////////////////
// functions declarations
void PrintInstruction();
void MainLoop();
bool OpenCAN();
void CloseCAN();
void DriveReset();
void DriveInit();
void DriveOff();
void ProcessCANMessage(int index);
void StartCANListenThread();
void StopCANListenThread();
#ifdef PeakCAN
extern "C" int getPCANChannelIndex(const char* cname);
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// device control
void SetModeOfOperation();
void SetTargetPosition();
void ReadyToSwitchOn();
void SwitchedOn();
void OperationEnable();
void Shutdown();
void StartHoming();

/////////////////////////////////////////////////////////////////////////////////////////
// motion declarations
void MotionStretch();
void MotionSquat();
void MotionWalkReady();
void MotionWalk();

/////////////////////////////////////////////////////////////////////////////////////////
// CAN message dispatcher
void ProcessCANMessage(int index)
{
	unsigned char fn_code;
	unsigned char node_id;
	//unsigned short obj_index;
	//unsigned char sub_index;
	unsigned char len;
	unsigned char data[8];
	unsigned char data_return = 0;
	//int i;

	while (0 == can_get_message(CAN_Ch[index], fn_code, node_id, len, data, false))
	{
		switch (fn_code)
		{
		case COBTYPE_TxSDO:
			{
			}
			break;

		case COBTYPE_TxPDO1:
			{
				printf("\tTxPDO1[node=%d]: ", node_id);
				printf("status word = ");
				printBinary(data[1]);
				printf(" ");
				printBinary(data[0]);
				printf("b\n");
			}
			break;
		case COBTYPE_TxPDO2:
			{
				printf("\tTxPDO2[node=%d]: ", node_id);
				printf("\n");
			}
			break;
		case COBTYPE_TxPDO3:
			{
				printf("\tTxPDO3[node=%d]: ", node_id);
				printf("position = %d", MAKELONG(MAKEWORD(data[0], data[1]), MAKEWORD(data[2], data[3])));
				printf(", DI = ");
				printBinary(data[6]);
				printf(" ");
				printBinary(data[5]);
				printf(" ");
				printBinary(data[4]);
				/*printf(" ");
				printBinary(data[4]);*/
				printf("b\n");
			}
			break;
		case COBTYPE_TxPDO4:
			{
				printf("\tTxPDO4[node=%d]: ", node_id);
				printf("\n");
			}
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// CAN communication thread
static unsigned int __stdcall ioThreadProc(void* inst)
{
	int can_index = (int)inst;

	while (ioThreadRun)
	{
		ProcessCANMessage(can_index);
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
			for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
			{
				if (!CAN_Ch_Enabled[ch]) continue;
				ProcessCANMessage(ch);
			}
			Sleep(5);
			sync_counter++;
			if (sync_counter == 100) {
				
				for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
				{
					if (!CAN_Ch_Enabled[ch]) continue;
					for (int node = 0; node < NODE_COUNT; node++)
					{
						if (!NODE_Enabled[ch][node]) continue;
//						can_pdo_rx3(CAN_Ch[ch], JointNodeID[ch][node], targetPosition, targetVelocity);
						can_pdo_rx1(CAN_Ch[ch], JointNodeID[ch][node], controlWord, modeOfOperation);
					}

				}
				
//				controlWord &= 0xFF8F; // masking irrelevant bits
//				controlWord |= 0x00; // clear all operation mode specific bits

				for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
				{
					if (!CAN_Ch_Enabled[ch]) continue;
					can_sync(CAN_Ch[ch]);
				}

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
				{
					for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
					{
						if (!CAN_Ch_Enabled[ch]) continue;
						can_sync(CAN_Ch[ch]);
					}
				}
				break;
			
			case '1':
				SetModeOfOperation();
				break;

			case '2':
				ReadyToSwitchOn();
				break;

			case '3':
				SwitchedOn();
				break;

			case '4':
				OperationEnable();
				break;

			case '5':
				StartHoming();
				break;

			case '6':
				Shutdown();
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
	CAN_Ch[0] = getPCANChannelIndex("PCAN_PCIBUS1");
	CAN_Ch[1] = getPCANChannelIndex("PCAN_PCIBUS2");
	CAN_Ch[2] = getPCANChannelIndex("PCAN_PCIBUS3");
	CAN_Ch[3] = getPCANChannelIndex("PCAN_PCIBUS4");
#elif defined(IXXATCAN) || defined(SOFTINGCAN)
	CAN_Ch[0] = 1;
	CAN_Ch[1] = 1;
	CAN_Ch[2] = 1;
	CAN_Ch[3] = 1;
#elif defined(NICAN)
	CAN_Ch[0] = 0;
	CAN_Ch[1] = 1;
	CAN_Ch[2] = 2;
	CAN_Ch[3] = 3;
#else
	CAN_Ch[0] = 1;
	CAN_Ch[1] = 2;
	CAN_Ch[2] = 3;
	CAN_Ch[3] = 4;
#endif

	for (int ch = 0; ch < CAN_Ch_COUNT; ch++) {
		if (!CAN_Ch_Enabled[ch]) continue;
		printf(">CAN(%d): open\n", CAN_Ch[ch]);
		ret = can_open(CAN_Ch[ch]);
		if(ret < 0)
		{
			printf("ERROR command_canopen !!! \n");
			return false;
		}
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

	for (int ch = 0; ch < CAN_Ch_COUNT; ch++) {
		if (!CAN_Ch_Enabled[ch]) continue;
		printf(">CAN(%d): close\n", CAN_Ch[ch]);
		ret = can_close(CAN_Ch[ch]);
		if(ret < 0) printf("ERROR command_can_close !!! \n");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Reset motor drives
void DriveReset()
{
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
			// reset device:
			printf("reset node(%d, %d)...\n", CAN_Ch[ch], JointNodeID[ch][node]);
			can_nmt_soft_reset(CAN_Ch[ch], JointNodeID[ch][node]);
		}
	}
	Sleep(1000);
	printf("done.\n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Init motor drives
void DriveInit()
{
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;

		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;

			// PDO mapping:
			printf("PDO mapping...\n");
			can_map_rxpdo1(CAN_Ch[ch], JointNodeID[ch][node]);
//			can_map_rxpdo3(CAN_Ch[ch], JointNodeID[ch][node]);
			can_map_txpdo1(CAN_Ch[ch], JointNodeID[ch][node]);
			can_map_txpdo3(CAN_Ch[ch], JointNodeID[ch][node]);

			// set homing parameters:
			can_set_homing_params(CAN_Ch[ch], JointNodeID[ch][node], 0, 3, 40960*2, 4096*2, 60000000);
			can_dump_homing_params(CAN_Ch[ch], JointNodeID[ch][node]);

			// set mode of operation:
//			printf("set mode of operation...\n");
//			can_set_mode_of_operation(CAN_Ch[ch], JointNodeID[ch][node], OP_MODE_PROFILED_POSITION);
//			printf("query mode of operation...\n");
//			can_query_mode_of_operation_display(CAN_Ch[ch], JointNodeID[ch][node]);

			// set communication mode OPERATIONAL:
			printf("set communication mode OPERATIONAL...\n");
			can_nmt_node_start(CAN_Ch[ch], JointNodeID[ch][node]);

			// set unit mode:
			printf("set unit mode...\n");
			can_bin_set_unit_mode(CAN_Ch[ch], JointNodeID[ch][node], UM_POSITION);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Turn off motor drives
void DriveOff()
{
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;

		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;

			// set communication mode PREPARED:
			printf("set communication mode STOPPED...\n");
			can_nmt_node_stop(CAN_Ch[ch], JointNodeID[ch][node]);

			// flush can messages:
			printf("flush can messages...\n");
			can_flush(CAN_Ch[ch], JointNodeID[ch][node]);

			// servo off:
			//printf("servo off...\n");
			//can_servo_off(CAN_Ch, NODE_ID, controlWord);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Start/Stop CAN message listener
void StartCANListenThread()
{
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;

		recvNum[ch] = 0;
		sendNum[ch] = 0;
		statTime[ch] = 0.0;

		ioThreadRun[ch] = true;
		ioThread[ch] = _beginthreadex(NULL, 0, ioThreadProc,(void*)(ch), 0, NULL);
		printf(">CAN(%d): starts listening CAN frames\n", CAN_Ch[ch]);
	}
}

void StopCANListenThread()
{
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;

		if (ioThreadRun[ch])
		{
			printf(">CAN(%d): stoped listening CAN frames\n", CAN_Ch[ch]);
			ioThreadRun[ch] = false;
			WaitForSingleObject((HANDLE)ioThread[ch], INFINITE);
			CloseHandle((HANDLE)ioThread[ch]);
			ioThread[ch] = 0;
		}
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
// device control:
void SetModeOfOperation()
{
	printf("set mode of operation...\n");
	modeOfOperation = OP_MODE_HOMING;
}

void SetTargetPosition()
{
	printf("set target position...\n");
	targetPosition = 10000;
	targetVelocity = 409600;
	controlWord &= 0xFF8F; // masking irrelevant bits
	//controlWord |= 0x2070; // set new point, target position is relative
	controlWord |= 0x0070; // set new point, target position is relative
}

void ReadyToSwitchOn()
{
	printf("ready to switch on...\n");
	controlWord &= 0xFF78; // masking irrelevant bits
	controlWord |= 0x06;
}

void SwitchedOn()
{
	printf("switched on...\n");
	controlWord &= 0xFF70; // masking irrelevant bits
	controlWord |= 0x07;
}

void OperationEnable()
{
	printf("operation enable...\n");
	controlWord &= 0xFF70; // masking irrelevant bits
	controlWord |= 0x0F;
}

void Shutdown()
{
	printf("shutdown...\n");
	//controlWord &= 0xFF7D; // masking irrelevant bits
	//controlWord |= 0x00;
	controlWord = 0x00;
}

void StartHoming()
{
	printf("start homing...\n");
	controlWord &= 0xFFEF; // masking irrelevant bits
	controlWord |= 0x10;
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

	DriveReset();
	DriveInit();

	// start periodic communication:
//	printf("start periodic communication...\n");
//	StartCANListenThread();

	/*SetModeOfOperation();
	Sleep(50);
	ReadyToSwitchOn();
	Sleep(50);
	SwitchedOn();
	Sleep(50);
	OperationEnable();
	Sleep(50);*/
	/*Shutdown();
	Sleep(50);*/

	// loop wait user input:
	printf("main loop...\n");
	MainLoop();

	// stop periodic communication:
//	printf("stop periodic communication...\n");
//	StopCANListenThread();
	
	DriveOff();

	// close CAN channel:
	CloseCAN();

	return 0;
}
