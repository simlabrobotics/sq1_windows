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
const bool CAN_Ch_Enabled[CAN_Ch_COUNT] = {true, true, true, true};
const bool NODE_Enabled[LEG_COUNT][LEG_JDOF] = {
	{true, true, true},
	{true, true, true},
	{true, true, true},
	{true, true, true}
};
bool ioThreadRun[CAN_Ch_COUNT] = {false, false, false, false};
uintptr_t ioThread[CAN_Ch_COUNT] = {0, 0, 0, 0};
int recvNum[CAN_Ch_COUNT] = {0, 0, 0, 0};
int sendNum[CAN_Ch_COUNT] = {0, 0, 0, 0};
double statTime[CAN_Ch_COUNT] = {-1.0, -1.0, -1.0, -1.0};
sQ1_RobotMemory_t vars;
long targetPosition[LEG_COUNT][LEG_JDOF];
unsigned long targetVelocity[LEG_COUNT][LEG_JDOF];
long homingStatus[LEG_COUNT][LEG_JDOF];
const long homingOffset[LEG_COUNT][LEG_JDOF] = {
	{0, -DEG2COUNT(80)-20000,  DEG2COUNT(80)+30000},
	{0,  DEG2COUNT(80)+10000, -DEG2COUNT(80)-40000},
	{0, -DEG2COUNT(80)-30000, -DEG2COUNT(80)-30000},
	{0,  DEG2COUNT(80)+50000,  DEG2COUNT(80)+30000}
};
const char homingMethod[LEG_COUNT][LEG_JDOF] = {
	{HM_CURRENT_POSITION, HM_POSHOMESW_INDEXPULSE_N, HM_NEGHOMESW_INDEXPULSE_N},
	{HM_CURRENT_POSITION, HM_NEGHOMESW_INDEXPULSE_N, HM_POSHOMESW_INDEXPULSE_N},
	{HM_CURRENT_POSITION, HM_POSHOMESW_INDEXPULSE_N, HM_POSHOMESW_INDEXPULSE_N},
	{HM_CURRENT_POSITION, HM_NEGHOMESW_INDEXPULSE_N, HM_NEGHOMESW_INDEXPULSE_N}
};
long motionStatus[LEG_COUNT][LEG_JDOF];
unsigned char modeOfOperation = OP_MODE_PROFILED_POSITION;
unsigned short controlWord[LEG_COUNT][LEG_JDOF];
unsigned short statusWord[LEG_COUNT][LEG_JDOF];

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
void InitVariables();
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
void EStop();
void StartHoming();
long GetHomingDone();
void UpdateStatus(int ch, unsigned char node_id, unsigned short status_word);

/////////////////////////////////////////////////////////////////////////////////////////
// motion declarations
void MotionZero();
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
				UpdateStatus(index, node_id, MAKEWORD(data[4], data[5]));
				printf("\tTxPDO1[node=%d]\n", node_id);
				printf("\t\tstatus word = ");
				printBinary(data[5]);
				printf(" ");
				printBinary(data[4]);
				printf("b\n");
				printf("\t\tDI = ");
				printBinary(data[2]);
				printf(" ");
				printBinary(data[1]);
				printf(" ");
				printBinary(data[0]);
				printf("b\n");
			}
			break;
		case COBTYPE_TxPDO2:
			{
				printf("\tTxPDO2[node=%d]: ", node_id);
				printf("%c%c[%d] = ", data[0], data[1], (data[2] | ((unsigned short)(data[3]&0x3F)<<8)));
				if ((data[3]&0x40) != 0) {
					printf("ERROR(%04X %04Xh)", MAKEWORD(data[6], data[7]), MAKEWORD(data[4], data[5]));
				}
				else {
					if ((data[3]&0x80) != 0) {
						printf("%f", (float)(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7])));
					}
					else {
						printf("%d", (int)(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7])));
					}
				}
				printf("\n");
			}
			break;
		case COBTYPE_TxPDO3:
			{
				long enc_counter;
				printf("\tTxPDO3[node=%d]\n", node_id);
				enc_counter = MAKELONG(MAKEWORD(data[0], data[1]), MAKEWORD(data[2], data[3]));
				printf("\t\tdemand position = %.1f (deg) / %d (count)\n", COUNT2DEG(enc_counter), enc_counter);
				enc_counter = MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]));
				printf("\t\tactual position = %.1f (deg) / %d (count)\n", COUNT2DEG(enc_counter), enc_counter);
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
			sync_counter++;

			if (sync_counter == 1) {
				
				for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
				{
					if (!CAN_Ch_Enabled[ch]) continue;
					for (int node = 0; node < NODE_COUNT; node++)
					{
						if (!NODE_Enabled[ch][node]) continue;

						if (GetHomingDone() == HOMING_DONE &&
							(statusWord[ch][node]&0x1000) != 0 &&
							(controlWord[ch][node]&0x0010) != 0) { // when "Set new point" bit is set...
							controlWord[ch][node] &= 0xDF8F; // masking irrelevant bits
							controlWord[ch][node] |= 0x0000; // clear all operation mode specific bits
						}
						
						can_pdo_rx1(CAN_Ch[ch], JointNodeID[ch][node], targetPosition[ch][node], targetVelocity[ch][node]);
						can_pdo_rx3(CAN_Ch[ch], JointNodeID[ch][node], controlWord[ch][node], modeOfOperation);
					}
				}
				
				for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
				{
					if (!CAN_Ch_Enabled[ch]) continue;
					can_sync(CAN_Ch[ch]);
				}

				sync_counter = 0;
			}

			Sleep(10);

			for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
			{
				if (!CAN_Ch_Enabled[ch]) continue;
				ProcessCANMessage(ch);
			}
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

			case '9':
				Shutdown();
				break;

			case 'h': case 'H':
				StartHoming();
				break;

			default:
				{ 
					//
					// process motion command:
					//
					if (GetHomingDone() != HOMING_DONE) {
						printf("Cannot process a motion command. Homing has not been done yet!\n");
						break;
					}

					switch (c)
					{
					case 'z': case 'Z':
						MotionZero();
						break;

					case 'u': case 'U':
						MotionStretch();
						break;

					case 'd': case 'D':
						MotionSquat();
						break;

					case 'r': case 'R':
						MotionWalkReady();
						break;

					case 'w': case 'W':
						MotionWalk();
						break;

					case 'e': case 'E':
						EStop();
						break;

					case '5': // testing incremental position control
						SetTargetPosition();
						break;
					}
				}
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
	CAN_Ch[1] = 2;
	CAN_Ch[2] = 3;
	CAN_Ch[3] = 4;
#else defined(NICAN) || defined(ESDCAN)
	CAN_Ch[0] = 0;
	CAN_Ch[1] = 1;
	CAN_Ch[2] = 2;
	CAN_Ch[3] = 3;
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
			can_map_rxpdo3(CAN_Ch[ch], JointNodeID[ch][node]);
			can_map_txpdo1(CAN_Ch[ch], JointNodeID[ch][node]);
			can_map_txpdo3(CAN_Ch[ch], JointNodeID[ch][node]);

			// set homing parameters:
			can_set_homing_params(CAN_Ch[ch], JointNodeID[ch][node], homingOffset[ch][node], homingMethod[ch][node], DEG2COUNT(5), DEG2COUNT(1), 60000000);
			can_dump_homing_params(CAN_Ch[ch], JointNodeID[ch][node]);

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
	printf("1: Set profiled position control mode\n");
	printf("2: Ready to switch on\n");
	printf("3: Switch on\n");
	printf("4: Operation enable\n");
	printf("9: Shutdown\n");
	printf("\n");

	printf("H: Homing\n");
	printf("\n");
	
	printf("Z: Move to Zero position\n");
	printf("U: Stretch All Legs (stand UP)\n");
	printf("D: Squat Motion (sit DOWN)\n");	
	printf("R: Walk-Ready Position (not implemented yet.)\n");
	printf("W: Start Walk (not implemented yet.)\n");
	printf("\n");

	printf("S: Update encoder & DI values\n");
	printf("E: E-STOP\n");
	printf("Q: Quit this program\n");

	printf("--------------------------------------------------\n\n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Initialize global variables
void InitVariables()
{
	memset(&vars, 0, sizeof(vars));
	memset(controlWord, 0, LEG_COUNT*LEG_JDOF*sizeof(controlWord[0][0]));
	memset(statusWord, 0, LEG_COUNT*LEG_JDOF*sizeof(statusWord[0][0]));
	memset(targetPosition, 0, LEG_COUNT*LEG_JDOF*sizeof(targetPosition[0][0]));
	memset(targetVelocity, 0, LEG_COUNT*LEG_JDOF*sizeof(targetVelocity[0][0]));
	memset(motionStatus, 0, LEG_COUNT*LEG_JDOF*sizeof(motionStatus[0][0]));
	memset(homingStatus, 0, LEG_COUNT*LEG_JDOF*sizeof(homingStatus[0][0]));
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++) {
		for (int node = 0; node < NODE_COUNT; node++)  {
			if (node == 0) homingStatus[ch][node] = HOMING_DONE;
			else homingStatus[ch][node] = HOMING_DONE;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// device control:
void SetModeOfOperation()
{
	printf("set mode of operation(PROFILED_POSITION)...\n");
	modeOfOperation = OP_MODE_PROFILED_POSITION;
}

void ReadyToSwitchOn()
{
	printf("ready to switch on...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			controlWord[ch][node] &= 0xFF78; // masking irrelevant bits
			controlWord[ch][node] |= 0x06;
		}
	}
}

void SwitchedOn()
{
	printf("switched on...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			controlWord[ch][node] &= 0xFF70; // masking irrelevant bits
			controlWord[ch][node] |= 0x07;
		}
	}
}

void OperationEnable()
{
	printf("operation enable...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			controlWord[ch][node] &= 0xFF70; // masking irrelevant bits
			controlWord[ch][node] |= 0x0F;
		}
	}
}

void Shutdown()
{
	printf("shutdown...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			//controlWord[ch][node] &= 0xFF7D; // masking irrelevant bits
			//controlWord[ch][node] |= 0x00;
			controlWord[ch][node] = 0x00;
		}
	}
}

void EStop()
{
	printf("E-Stop...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			//controlWord[ch][node] &= 0xFF7D; // masking irrelevant bits
			//controlWord[ch][node] |= 0x00;
			controlWord[ch][node] = 0x00;
		}
	}
}

void StartHoming()
{
	printf("start homing...\n");

	printf("set mode of operation(HOMING)...\n");
	modeOfOperation = OP_MODE_HOMING;

	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			controlWord[ch][node] &= 0xFFEF; // masking irrelevant bits
			controlWord[ch][node] |= 0x10;
			homingStatus[ch][node] = HOMING_INIT;
		}
	}
}

long GetHomingDone()
{
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			if (homingStatus[ch][node] != HOMING_DONE)
				return homingStatus[ch][node];
		}
	}
	return HOMING_DONE;
}

void UpdateStatus(int ch, unsigned char node_id, unsigned short status_word)
{
	if (!CAN_Ch_Enabled[ch]) return;
	for (int i=0; i<LEG_JDOF; i++)
	{
		if (!NODE_Enabled[ch][i]) continue;

		if (JointNodeID[ch][i] == node_id)
		{
			statusWord[ch][i] = status_word;

			// homing is done?
			if (homingStatus[ch][i] == HOMING_INIT) {
				if ((statusWord[ch][i] & 0x2000) != 0) {
					printf("\thoming ERROR (CAN ch=%d, node id=%d)\n", ch, node_id);
					homingStatus[ch][i] = HOMING_ERROR;
					controlWord[ch][i] &= 0xFEEF; // masking irrelevant bits
					controlWord[ch][i] |= 0x0100; // clear all operation mode specific bits and set Halt bit on
				}
				else if ((statusWord[ch][i] & 0x1000) != 0) {
					printf("\thoming done (CAN ch=%d, node id=%d)\n", ch, node_id);
					homingStatus[ch][i] = HOMING_DONE;
					controlWord[ch][i] &= 0xFFEF; // masking irrelevant bits
					controlWord[ch][i] |= 0x00; // clear all operation mode specific bits

					if (GetHomingDone() == HOMING_DONE) {
						printf("HOMING for all joints has been completed!\n");
						SetModeOfOperation();
					}
				}
			}

			return;
		}
	}
}

void SetTargetPosition()
{
	printf("set target position...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;

			targetPosition[ch][node] = DEG2COUNT(5);
			targetVelocity[ch][node] = DEG2COUNT(10);

			controlWord[ch][node] &= 0xDF8F; // masking irrelevant bits
			//controlWord[ch][node] |= 0x2070; // set new point, target position is relative
			controlWord[ch][node] |= 0x2070; // set new point, target position is relative
		}
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////
// Demo motions:
void MotionZero()
{
	printf("move to ZERO position...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			targetPosition[ch][node] = DEG2COUNT(0);
			targetVelocity[ch][node] = DEG2COUNT(10);

			controlWord[ch][node] &= 0xDF8F; // masking irrelevant bits
			controlWord[ch][node] |= 0x2030; // set new point, target position is absolute
		}
	}
}

void MotionStretch()
{
	static const unsigned long target_velocity[LEG_COUNT][LEG_JDOF] = {
		{DEG2COUNT(0), DEG2COUNT(20), DEG2COUNT(30)},
		{DEG2COUNT(0), DEG2COUNT(20), DEG2COUNT(30)},
		{DEG2COUNT(0), DEG2COUNT(20), DEG2COUNT(30)},
		{DEG2COUNT(0), DEG2COUNT(20), DEG2COUNT(30)}
	};

	printf("move to STRETCH position...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			targetPosition[ch][node] = DEG2COUNT(0);
			targetVelocity[ch][node] = target_velocity[ch][node];

			controlWord[ch][node] &= 0xDF8F; // masking irrelevant bits
			controlWord[ch][node] |= 0x2030; // set new point, target position is absolute
		}
	}
}

void MotionSquat()
{
	static const long target_position[LEG_COUNT][LEG_JDOF] = {
		{DEG2COUNT(0), -DEG2COUNT(40), -DEG2COUNT(60)},
		{DEG2COUNT(0),  DEG2COUNT(40),  DEG2COUNT(60)},
		{DEG2COUNT(0), -DEG2COUNT(40), -DEG2COUNT(60)},
		{DEG2COUNT(0),  DEG2COUNT(40),  DEG2COUNT(60)}
	};
	static const unsigned long target_velocity[LEG_COUNT][LEG_JDOF] = {
		{DEG2COUNT(0), DEG2COUNT(20), DEG2COUNT(30)},
		{DEG2COUNT(0), DEG2COUNT(20), DEG2COUNT(30)},
		{DEG2COUNT(0), DEG2COUNT(20), DEG2COUNT(30)},
		{DEG2COUNT(0), DEG2COUNT(20), DEG2COUNT(30)}
	};

	printf("move to SQUAT position...\n");
	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;
	
			targetPosition[ch][node] = target_position[ch][node];
			targetVelocity[ch][node] = target_velocity[ch][node];

			controlWord[ch][node] &= 0xDF8F; // masking irrelevant bits
			controlWord[ch][node] |= 0x2030; // set new point, target position is absolute
		}
	}
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
	InitVariables();

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
