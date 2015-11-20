/* mySQ1.cpp : Defines the entry point for the console application.
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
unsigned char NODE_ID = 0x01;
bool ioThreadRun = false;
uintptr_t ioThread = 0;
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
			
		}
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Application main-loop. It handles the keyboard events
void MainLoop()
{
	bool bRun = true;

	while (bRun)
	{
		if (!_kbhit())
		{
			Sleep(5);
			/*if (pSHM)
			{
				switch (pSHM->cmd.command)
				{
				case CMD_SERVO_ON:
					break;
				case CMD_SERVO_OFF:
					if (pBHand) pBHand->SetMotionType(eMotionType_NONE);
					break;
				case CMD_CMD_1:
					if (pBHand) pBHand->SetMotionType(eMotionType_HOME);
					break;
				case CMD_CMD_2:
					if (pBHand) pBHand->SetMotionType(eMotionType_READY);
					break;
				case CMD_CMD_3:
					if (pBHand) pBHand->SetMotionType(eMotionType_GRASP_3);
					break;
				case CMD_CMD_4:
					if (pBHand) pBHand->SetMotionType(eMotionType_GRASP_4);
					break;
				case CMD_CMD_5:
					if (pBHand) pBHand->SetMotionType(eMotionType_PINCH_IT);
					break;
				case CMD_CMD_6:
					if (pBHand) pBHand->SetMotionType(eMotionType_PINCH_MT);
					break;
				case CMD_CMD_7:
					if (pBHand) pBHand->SetMotionType(eMotionType_ENVELOP);
					break;
				case CMD_CMD_8:
					if (pBHand) pBHand->SetMotionType(eMotionType_GRAVITY_COMP);
					break;
				case CMD_EXIT:
					bRun = false;
					break;
				}
				pSHM->cmd.command = CMD_NULL;
				for (i=0; i<MAX_DOF; i++)
				{
					pSHM->state.slave_state[i].position = q[i];
					pSHM->cmd.slave_command[i].torque = tau_des[i];
				}
				pSHM->state.time = curTime;
			}*/
		}
		else
		{
			int c = _getch();
			switch (c)
			{
			case 'q':
				bRun = false;
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

	recvNum = 0;
	sendNum = 0;
	statTime = 0.0;

	ioThreadRun = true;
	ioThread = _beginthreadex(NULL, 0, ioThreadProc, NULL, 0, NULL);
	printf(">CAN: starts listening CAN frames\n");
	
	printf(">CAN: query system id\n");
	ret = can_query_node_id(CAN_Ch, NODE_ID);
	if(ret < 0)
	{
		printf("ERROR command_can_query_id !!! \n");
		can_close(CAN_Ch);
		return false;
	}

	printf(">CAN: system init\n");
	ret = can_sys_init(CAN_Ch, NODE_ID, 5/*msec*/);
	if(ret < 0)
	{
		printf("ERROR command_can_sys_init !!! \n");
		can_close(CAN_Ch);
		return false;
	}

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
	printf("mySQ1: ");

	printf("Keyboard Commands:\n");
	printf("1: Stretch All Legs Downwards\n");
	printf("2: Squat Motion\n");	
	printf("3: Walk-Ready Position\n");
	printf("4: Start Walk\n");

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

	if (OpenCAN())
		MainLoop();

	CloseCAN();

	return 0;
}
