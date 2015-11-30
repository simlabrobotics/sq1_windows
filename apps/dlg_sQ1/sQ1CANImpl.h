/* app_SQ1 : Defines the entry point for the console application.
 *
 * Copyright (c) 2016 SimLab Co., Ltd. http://www.simlab.co.kr/
 * 
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose, 
 * including commercial applications, and to alter it and redistribute it freely. 
 */
#ifndef __SQ1CANIMPL_H__
#define __SQ1CANIMPL_H__

#include "sq1_def.h"

NAMESPACE_SQ1_BEGIN

/////////////////////////////////////////////////////////////////////////////////////////
// for CAN communication
extern unsigned int CAN_Ch_COUNT;
extern unsigned int NODE_COUNT;
extern int CAN_Ch[LEG_COUNT];
extern bool CAN_Ch_Enabled[LEG_COUNT];
extern bool NODE_Enabled[LEG_COUNT][LEG_JDOF];
extern long targetPosition[LEG_COUNT][LEG_JDOF];
extern unsigned long targetVelocity[LEG_COUNT][LEG_JDOF];
extern long homingStatus[LEG_COUNT][LEG_JDOF];
extern long motionStatus[LEG_COUNT][LEG_JDOF];
extern unsigned char modeOfOperation;
extern unsigned short controlWord[LEG_COUNT][LEG_JDOF];
extern unsigned short statusWord[LEG_COUNT][LEG_JDOF];

/////////////////////////////////////////////////////////////////////////////////////////
// functions declarations
void InitVariables();
bool OpenCAN();
void CloseCAN();
void DriveReset();
void DriveInit();
void DriveOff();
void ProcessCANMessage(int index);
void StartCANListenThread();
void StopCANListenThread();

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

NAMESPACE_SQ1_END

#endif
