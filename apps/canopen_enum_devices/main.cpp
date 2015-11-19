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
const double delT = 0.003;
int CAN_Ch = 0;
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
	char id_des;
	char id_cmd;
	char id_src;
	int len;
	unsigned char data[8];
	unsigned char data_return = 0;
	int i;

	while (ioThreadRun)
	{
		while (0 == get_message(CAN_Ch, &id_cmd, &id_src, &id_des, &len, data, FALSE))
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
	ret = command_can_open(CAN_Ch);
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
	ret = command_can_query_id(CAN_Ch);
	if(ret < 0)
	{
		printf("ERROR command_can_query_id !!! \n");
		command_can_close(CAN_Ch);
		return false;
	}

	printf(">CAN: system init\n");
	ret = command_can_sys_init(CAN_Ch, 3/*msec*/);
	if(ret < 0)
	{
		printf("ERROR command_can_sys_init !!! \n");
		command_can_close(CAN_Ch);
		return false;
	}

	printf(">CAN: start periodic communication\n");
	ret = command_can_start(CAN_Ch);
	if(ret < 0)
	{
		printf("ERROR command_can_start !!! \n");
		command_can_stop(CAN_Ch);
		command_can_close(CAN_Ch);
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Close CAN data channel
void CloseCAN()
{
	int ret;

	printf(">CAN: stop periodic communication\n");
	ret = command_can_stop(CAN_Ch);
	if(ret < 0)
	{
		printf("ERROR command_can_stop !!! \n");
	}

	if (ioThreadRun)
	{
		printf(">CAN: stoped listening CAN frames\n");
		ioThreadRun = false;
		WaitForSingleObject((HANDLE)ioThread, INFINITE);
		CloseHandle((HANDLE)ioThread);
		ioThread = 0;
	}

	printf(">CAN(%d): close\n", CAN_Ch);
	ret = command_can_close(CAN_Ch);
	if(ret < 0) printf("ERROR command_can_close !!! \n");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Print program information and keyboard instructions
void PrintInstruction()
{
	printf("--------------------------------------------------\n");
	printf("enum devices: \n");
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

	// switch all slaves to Configuration mode:
	lss_switch_mode(CAN_Ch, 0x01);

	// LSS identify remote slaves

	// switch all slaves to Operation mode:
	lss_switch_mode(CAN_Ch, 0x00);

	// close CAN channel:
	CloseCAN();

	return 0;
}
