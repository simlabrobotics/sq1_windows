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
unsigned char selectedNode = NODEID_GLOBAL;

static void printBinary(unsigned char by)
{
	char sz[9];
	int i;
	sz[0] = '\0';
	for (i=128; i>0; i>>=1)
		printf("%c", (by&i?'1':'0'));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interprete user command
void ProcessCmd(char* szCmd);

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
						printf("%f", (float)(MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))));
					}
					else {
						printf("%d", (int)(MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))));
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
// Interprete user command
void ProcessCmd(char* szCmd)
{
	long cmd_len;
	long cmd_index;
	long cmd_rval_i;
	float cmd_rval_f;
	long cmd_type;
	int cmd_ptr;
	const char* index_ptr;
	const char* rval_ptr;

	printf("parsing commdnd ( %s )...\n", szCmd);
	cmd_len = strlen((const char*)szCmd);
	cmd_index = 0;
	cmd_ptr = 0;

	/*
	UM=1
	MO=1
	SP=100000
	PR=200000
	BG
	MO=0
	*/

	// to upper case:
	cmd_ptr = 0;
	while (szCmd[cmd_ptr] != '\0') {
		if (szCmd[cmd_ptr] >= 'a' && szCmd[cmd_ptr] <= 'z')
			szCmd[cmd_ptr] += ('A' - 'a');
		cmd_ptr++;
	}

	// parse command:
	cmd_ptr = 2;

	if (szCmd[cmd_ptr] == '[') {
		szCmd[cmd_ptr++] = '\0';
		index_ptr = (const char*)(szCmd+cmd_ptr);

		while (szCmd[cmd_ptr] != '\0') {
			if (szCmd[cmd_ptr] == ']') {
				break;
			}
			cmd_ptr++;
		}
		if (cmd_ptr >= cmd_len) {
			printf("Failed. [] is mis-matching.\n");
			return;
		}

		szCmd[cmd_ptr++] = '\0';
		cmd_index = atoi(index_ptr);
	}
	else {
		cmd_index = 0;
	}

	if (cmd_ptr == cmd_len) {
		cmd_type = 1; // excute/query with index
	}
	else {
		if (szCmd[cmd_ptr] != '=' || (cmd_ptr+1 >= cmd_len)) {
			printf("Failed. RVal is missing.\n");
			return;
		}
		cmd_ptr++;

		rval_ptr = (const char*)(szCmd+cmd_ptr);

		while (szCmd[cmd_ptr] != '\0') {
			if (szCmd[cmd_ptr] == '.') {
				break;
			}
			cmd_ptr++;
		}
		if (cmd_ptr >= cmd_len) {
			cmd_type = 2;
			cmd_rval_i = atoi(rval_ptr);
		}
		else {
			cmd_type = 3;
			cmd_rval_f = (float)atof(rval_ptr);
		}
	}


	printf("OK!\n");

	switch (cmd_type)
	{
	case 1:
		printf("%c%c %d", szCmd[0], szCmd[1], cmd_index);
		break;
	case 2:
		printf("%c%c %d %d", szCmd[0], szCmd[1], cmd_index, cmd_rval_i);
		break;
	case 3:
		printf("%c%c %d %d", szCmd[0], szCmd[1], cmd_index, cmd_rval_f);
		break;
	}
	printf(";\n");

	for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		for (int node = 0; node < NODE_COUNT; node++) 
		{
			if (!NODE_Enabled[ch][node]) continue;

			if (NODEID_GLOBAL != selectedNode &&
				JointNodeID[ch][node] != selectedNode)
				continue;
			
			printf("send command to node(%d, %d)...\n", CAN_Ch[ch], JointNodeID[ch][node]);

			switch (cmd_type)
			{
			case 1:
				can_bin_interprete_get_i(CAN_Ch[ch], JointNodeID[ch][node], (unsigned char*)szCmd, (unsigned short)cmd_index);
				break;
			case 2:
				can_bin_interprete_set_i(CAN_Ch[ch], JointNodeID[ch][node], (unsigned char*)szCmd, (unsigned short)cmd_index, cmd_rval_i);
				break;
			case 3:
				can_bin_interprete_set_f(CAN_Ch[ch], JointNodeID[ch][node], (unsigned char*)szCmd, (unsigned short)cmd_index, cmd_rval_f);
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Application main-loop. It handles the keyboard events
void MainLoop()
{
	bool bRun = true;
	static int sync_counter = 0;
	char szCmd[256];
	int node_select;

	while (bRun)
	{
		/*if (!_kbhit())
		{
			for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
			{
				if (!CAN_Ch_Enabled[ch]) continue;
				ProcessCANMessage(ch);
			}
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
		}*/


		printf("%d> ", selectedNode);
		scanf_s("%s", szCmd, _countof(szCmd));

		if (!_stricmp(szCmd, "quit") || !_stricmp(szCmd, "exit")) {
			bRun = false;
		}
		else if (!_stricmp(szCmd, "node")) {
			printf("select node(0(all), 1~12) = ");
			scanf_s("%d", &node_select, 4);
			if (node_select >= 0 && node_select <= 12)
				selectedNode = (unsigned char)node_select;
		}
		else {
			if (strlen(szCmd) >= 2) {
				ProcessCmd(szCmd);

				Sleep(500);
				for (int ch = 0; ch < CAN_Ch_COUNT; ch++)
				{
					if (!CAN_Ch_Enabled[ch]) continue;
					ProcessCANMessage(ch);
				}
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

			// set communication mode OPERATIONAL:
			printf("set communication mode OPERATIONAL...\n");
			can_nmt_node_start(CAN_Ch[ch], JointNodeID[ch][node]);
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
	printf("sQ1 binary interpreter demo: ");

	printf("Keyboard Commands:\n");
	printf("quit or exit: Quit this program\n");
	printf("\n");
	printf("Exampels:\n");
	printf("UM=1\n");
	printf("MO=1\n");
	printf("SP=100000\n");
	printf("PR=200000\n");
	printf("BG\n");
	printf("MO=0\n");

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

//	DriveReset();
	DriveInit();

	// start periodic communication:
//	printf("start periodic communication...\n");
//	StartCANListenThread();

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
