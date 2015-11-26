/*
 *\brief API for communication over CAN bus 
 *\detailed The API for communicating with the various motor controllers
 *          over the CAN bus interface on the robot hand
 *
 *$Author: Sangyup Yi $
 *$Date: 2012/5/11 23:34:00 $
 *$Revision: 1.0 $
 */ 

#ifndef _CANAPI_H
#define _CANAPI_H

#include "canDef.h"

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif

/*=====================*/
/*       Defines       */
/*=====================*/
//constants
#define TX_QUEUE_SIZE       (32)
#define RX_QUEUE_SIZE       (32)
#define TX_TIMEOUT          (5)
#define RX_TIMEOUT          (5)
#define mbxID               (0)
#define BASE_ID             (0)
#define MAX_BUS             (256)

/******************/
/* CAN device API */
/******************/
CANAPI_BEGIN
CANAPI_EXTERN_C_BEGIN
int initCAN(int bus);
int freeCAN(int bus);
int canReadMsg(int bus, unsigned long &id, unsigned char &len, unsigned char *data, bool blocking);
int canSendMsg(int bus, unsigned long id, unsigned char len, unsigned char *data, bool blocking);
CANAPI_EXTERN_C_END
CANAPI_END

#endif
