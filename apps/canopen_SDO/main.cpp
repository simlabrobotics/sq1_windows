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
	unsigned char id_cmd;
	unsigned char id_src;
	int len;
	unsigned char data[8];
	unsigned char data_return = 0;
	int i;

	while (ioThreadRun)
	{
		while (0 == can_get_message(CAN_Ch, &id_cmd, &id_src, &len, data, FALSE))
		{
			switch (id_cmd)
			{
			//case ID_CMD_QUERY_ID:
			//	{
			//		printf(">CAN(%d): AllegroHand revision info: 0x%02x%02x\n", CAN_Ch, data[3], data[2]);
			//		printf("                      firmware info: 0x%02x%02x\n", data[5], data[4]);
			//		printf("                      hardware type: 0x%02x\n", data[7]);
			//	}
			//	break;

			//case ID_CMD_QUERY_CONTROL_DATA:
			//	{
			//		if (id_src >= ID_DEVICE_SUB_01 && id_src <= ID_DEVICE_SUB_04)
			//		{
			//			vars.enc_actual[(id_src-ID_DEVICE_SUB_01)*4 + 0] = (int)(data[0] | (data[1] << 8));
			//			vars.enc_actual[(id_src-ID_DEVICE_SUB_01)*4 + 1] = (int)(data[2] | (data[3] << 8));
			//			vars.enc_actual[(id_src-ID_DEVICE_SUB_01)*4 + 2] = (int)(data[4] | (data[5] << 8));
			//			vars.enc_actual[(id_src-ID_DEVICE_SUB_01)*4 + 3] = (int)(data[6] | (data[7] << 8));
			//			data_return |= (0x01 << (id_src-ID_DEVICE_SUB_01));
			//			recvNum++;
			//		}
			//		if (data_return == (0x01 | 0x02 | 0x04 | 0x08))
			//		{
			//			// convert encoder count to joint angle
			//			for (i=0; i<MAX_DOF; i++)
			//				q[i] = (double)(vars.enc_actual[i]*enc_dir[i]-32768-enc_offset[i])*(333.3/65536.0)*(3.141592/180.0);

			//			// compute joint torque
			//			ComputeTorque();

			//			// convert desired torque to desired current and PWM count
			//			for (i=0; i<MAX_DOF; i++)
			//			{
			//				cur_des[i] = tau_des[i] * motor_dir[i];
			//				if (cur_des[i] > 1.0) cur_des[i] = 1.0;
			//				else if (cur_des[i] < -1.0) cur_des[i] = -1.0;
			//			}

			//			// send torques
			//			for (int i=0; i<4;i++)
			//			{
			//				// the index order for motors is different from that of encoders

			//				switch (HAND_VERSION)
			//				{
			//					case 1:
			//					case 2:
			//						vars.pwm_demand[i*4+3] = (short)(cur_des[i*4+0]*tau_cov_const_v2);
			//						vars.pwm_demand[i*4+2] = (short)(cur_des[i*4+1]*tau_cov_const_v2);
			//						vars.pwm_demand[i*4+1] = (short)(cur_des[i*4+2]*tau_cov_const_v2);
			//						vars.pwm_demand[i*4+0] = (short)(cur_des[i*4+3]*tau_cov_const_v2);
			//						break;

			//					case 3:
			//					default:
			//						vars.pwm_demand[i*4+3] = (short)(cur_des[i*4+0]*tau_cov_const_v3);
			//						vars.pwm_demand[i*4+2] = (short)(cur_des[i*4+1]*tau_cov_const_v3);
			//						vars.pwm_demand[i*4+1] = (short)(cur_des[i*4+2]*tau_cov_const_v3);
			//						vars.pwm_demand[i*4+0] = (short)(cur_des[i*4+3]*tau_cov_const_v3);
			//						break;
			//				}

			//				if (DC_24V) {
			//					for (int j=0; j<4; j++) {
			//						if (vars.pwm_demand[i*4+j] > pwm_max_DC24V) vars.pwm_demand[i*4+j] = pwm_max_DC24V;
			//						else if (vars.pwm_demand[i*4+j] < -pwm_max_DC24V) vars.pwm_demand[i*4+j] = -pwm_max_DC24V;
			//					}
			//				} 
			//				else {
			//					for (int j=0; j<4; j++) {
			//						if (vars.pwm_demand[i*4+j] > pwm_max_DC8V) vars.pwm_demand[i*4+j] = pwm_max_DC8V;
			//						else if (vars.pwm_demand[i*4+j] < -pwm_max_DC8V) vars.pwm_demand[i*4+j] = -pwm_max_DC8V;
			//					}

			//				}

			//				write_current(CAN_Ch, i, &vars.pwm_demand[4*i]);
			//				for(int k=0; k<100000; k++);
			//			}
			//			sendNum++;
			//			curTime += delT;

			//			data_return = 0;
			//		}
			//	}
			//	break;
			default:
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

	// query Manufacturer's device name
	can_query_device_name(CAN_Ch, NODE_ID);

	// query H/W, S/W version
	can_query_hw_version(CAN_Ch, NODE_ID);
	can_query_sw_version(CAN_Ch, NODE_ID);

	// query LSS address
	can_query_lss_address(CAN_Ch, NODE_ID);

	// loop wait user input:
	MainLoop();

	// close CAN channel:
	CloseCAN();

	return 0;
}
