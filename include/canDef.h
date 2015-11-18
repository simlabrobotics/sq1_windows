/*
 *\brief Constants and enums for CAN communication
 *
 *$Author: Sangyup Yi $
 *$Date: 2015/11/18 19:00:00 $
 *$Revision: 1.0 $
 */ 

#ifndef _CANDEF_H
#define _CANDEF_H


#ifdef USING_NAMESPACE_CANAPI
#   define CANAPI_BEGIN namespace CANAPI {
#   define CANAPI_END };
#else
#   define CANAPI_BEGIN
#   define CANAPI_END
#endif



CANAPI_BEGIN


typedef struct{
	unsigned char	STD_EXT;
	unsigned long	msg_id;         // message identifier
   	unsigned char	data_length;    //
   	char			data[8];        // data array
} can_msg;

#define		STD		(bool)0
#define		EXT		(bool)1

///////////////////////////////////////////////
//  Define CAN master & slaves ID:
#define ID_DEVICE_MASTER	0x00
#define ID_DEVICE_SUB_01	0x01
#define ID_DEVICE_SUB_02	0x02
#define ID_DEVICE_SUB_03	0x03

////////////////////////////////////////////////
//  Define CAN Arbitration ID



////////////////////////////////////////////////


CANAPI_END

#endif
