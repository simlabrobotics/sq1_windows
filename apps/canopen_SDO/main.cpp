/* canopen_SDO: demonstrate CANopen SDO transmission.
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

/////////////////////////////////////////////////////////////////////////////////////////
// for CAN communication
const double delT = 0.005;
int CAN_Ch = 0;
const unsigned int NODE_COUNT = 3;
unsigned char NODE_ID[NODE_COUNT] = {0x0A, 0x0B, 0x0C};
int recvNum = 0;
int sendNum = 0;
double statTime = -1.0;

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
	unsigned short obj_index;
	unsigned char sub_index;
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
				unsigned char scs			= ((data[0] & 0xe0) >> 5);
				unsigned char num_of_bytes	= ((data[0] & 0x0c) >> 2);
				unsigned char tx_type		= ((data[0] & 0x02) >> 1);
				unsigned char size_indicator= ((data[0] & 0x01) >> 0);
				printf("\tscs=%d, num_of_bytes=%d, tx_type=%d, size_indicator=%d\n", scs, num_of_bytes, tx_type, size_indicator);

				obj_index = MAKEWORD(data[1], data[2]);
				sub_index = data[3];

				if (data[0] == 0x80/*scs == 0x04*/) // abort SDO transfer with error
				{
					printf("\tSDO transfer is aborted with error.\n");
					printf("\terror class = %04Xh\n", MAKEWORD(data[6], data[7]));
					printf("\terror code = %02Xh\n", data[5]);
					printf("\tadditional code = %02Xh\n", data[4]);
				}
				else
				{
					/*switch (obj_index)
					{
					case OD_DEVICE_TYPE:
						{
							printf("\tdevice profile number = %d\n", MAKEWORD(data[6], data[7]));
							printf("\tnumber of SDOs supported = %d\n", MAKEWORD(data[4], data[5]));
						}
						break;
					case OD_DEVICE_NAME:
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
							switch (sub_index)
							{
							case 1: printf("\tvendor id = %d\n", MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))); break;
							case 2: printf("\tproduct id = %d\n", MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))); break;
							case 3: printf("\trevision number = %d\n", MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))); break;
							case 4: printf("\tserial number = %d\n", MAKELONG(MAKEWORD(data[4], data[5]), MAKEWORD(data[6], data[7]))); break;
							}
						}
						break;
					case OD_RxPDO1_MAPPING:
					case OD_RxPDO2_MAPPING:
					case OD_RxPDO3_MAPPING:
					case OD_RxPDO4_MAPPING:
						{
							printf("\tRxPDO%d mapping[%d] = %04X\n", (obj_index-OD_RxPDO1_MAPPING+1), sub_index);
						}
						break;
					case OD_TxPDO1_MAPPING:
					case OD_TxPDO2_MAPPING:
					case OD_TxPDO3_MAPPING:
					case OD_TxPDO4_MAPPING:
						{
							printf("\tTxPDO%d mapping[%d] = %04X\n", (obj_index-OD_TxPDO1_MAPPING+1), sub_index);
						}
						break;
					}*/
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

/////////////////////////////////////////////////////////////////////////////////////////
// Application main-loop. It handles the keyboard events
void MainLoop()
{
	bool bRun = true;

	while (bRun)
	{
		if (!_kbhit())
		{
			ProcessCANMessage();
			Sleep(5);
		}
		else
		{
			int c = _getch();
			switch (c)
			{
			case 'q': case 'Q':
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
	CAN_Ch = getPCANChannelIndex("PCAN_PCIBUS4");
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
	printf("CANopen SDO: \n\n");

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

	for (int node_index=0; node_index<NODE_COUNT; node_index++)
	{
		// query device type:
		printf("query device type...\n");
		can_query_device_type(CAN_Ch, NODE_ID[node_index]);

		// query device name:
		printf("query device name...\n");
		can_query_device_name(CAN_Ch, NODE_ID[node_index]);

		// query node id:
		printf("query node id...\n");
		can_query_node_id(CAN_Ch, NODE_ID[node_index]);
	
		// query H/W, S/W version:
		printf("query H/W version...\n");
		can_query_hw_version(CAN_Ch, NODE_ID[node_index]);
		printf("query S/W version...\n");
		can_query_sw_version(CAN_Ch, NODE_ID[node_index]);
	
		// query supported drive modes:
		printf("query supported drive modes...\n");
		can_query_drive_modes(CAN_Ch, NODE_ID[node_index]);
	
		// set unit mode
		printf("set unit mode...\n");
		can_bin_set_unit_mode(CAN_Ch, NODE_ID[node_index], UM_POSITION);

		// query status register:
		printf("query status register...\n");
		can_query_status_register(CAN_Ch, NODE_ID[node_index]);

		// map PDO:
		printf("PDO mapping...\n");
		can_map_rxpdo1(CAN_Ch, NODE_ID[node_index]);
		can_map_rxpdo3(CAN_Ch, NODE_ID[node_index]);
		can_map_txpdo1(CAN_Ch, NODE_ID[node_index]);
		can_map_txpdo3(CAN_Ch, NODE_ID[node_index]);

		// query PDO mapping:
		printf("query RxPDO communication parameters...\n");
		can_query_RxPDO_params(CAN_Ch, NODE_ID[node_index], 1);
		can_query_RxPDO_params(CAN_Ch, NODE_ID[node_index], 2);
		can_query_RxPDO_params(CAN_Ch, NODE_ID[node_index], 3);
		can_query_RxPDO_params(CAN_Ch, NODE_ID[node_index], 4);

		printf("query RxPDO mapping...\n");
		can_query_RxPDO_mapping(CAN_Ch, NODE_ID[node_index], 1);
		can_query_RxPDO_mapping(CAN_Ch, NODE_ID[node_index], 2);
		can_query_RxPDO_mapping(CAN_Ch, NODE_ID[node_index], 3);
		can_query_RxPDO_mapping(CAN_Ch, NODE_ID[node_index], 4);
		
		printf("query TxPDO communication parameters...\n");
		can_query_TxPDO_params(CAN_Ch, NODE_ID[node_index], 1);
		can_query_TxPDO_params(CAN_Ch, NODE_ID[node_index], 2);
		can_query_TxPDO_params(CAN_Ch, NODE_ID[node_index], 3);
		can_query_TxPDO_params(CAN_Ch, NODE_ID[node_index], 4);

		printf("query TxPDO mapping...\n");
		can_query_TxPDO_mapping(CAN_Ch, NODE_ID[node_index], 1);
		can_query_TxPDO_mapping(CAN_Ch, NODE_ID[node_index], 2);
		can_query_TxPDO_mapping(CAN_Ch, NODE_ID[node_index], 3);
		can_query_TxPDO_mapping(CAN_Ch, NODE_ID[node_index], 4);

		// query LSS address:
		printf("query LSS address...\n");
		can_query_lss_address(CAN_Ch, NODE_ID[node_index]);

		// query control word:
		printf("query control word...\n");
		can_query_control_word(CAN_Ch, NODE_ID[node_index]);

		// query status word:
		printf("query status word...\n");
		can_query_status_word(CAN_Ch, NODE_ID[node_index]);

		// query mode of operation:
		printf("query mode of operation...\n");
		can_query_mode_of_operation_display(CAN_Ch, NODE_ID[node_index]);

		// query homing parameters:
		printf("query homing parameters...\n");
		can_dump_homing_params(CAN_Ch, NODE_ID[node_index]);

		// query unit factors:
		printf("query unit factors...\n");
		can_dump_factors(CAN_Ch, NODE_ID[node_index]);

		// query motion profile:
		printf("query motion profile...\n");
		can_dump_motion_profile(CAN_Ch, NODE_ID[node_index]);

		// query emergency events:
		printf("query emergency events...\n");
		can_query_emergency_events(CAN_Ch, NODE_ID[node_index]);
	}

	// loop wait user input:
	MainLoop();

	// close CAN channel:
	CloseCAN();

	return 0;
}
