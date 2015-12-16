/*======================*/
/*       Includes       */
/*======================*/
//system headers
#include <stdio.h>
#include <errno.h>
#ifndef _WIN32
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>
#endif
#include <windows.h>
#include <malloc.h>
#include <assert.h>

#include "canDef.h"
#include "canAPI.h"
#include "ESD/ntcan.h"

CANAPI_BEGIN
CANAPI_EXTERN_C_BEGIN

#define CH_COUNT			(int)4 // number of CAN channels

static NTCAN_HANDLE canDev[CH_COUNT] = { // CAN channel handles
	(NTCAN_HANDLE)-1,
	(NTCAN_HANDLE)-1,
	(NTCAN_HANDLE)-1,
	(NTCAN_HANDLE)-1
}; 

/*========================================*/
/*       Public functions (CAN API)       */
/*========================================*/
void allowMessage(int bus, int id, int mask){
    int i;
    DWORD retvalue;
	for(i=0;i<2048;i++){
		if((i & ~mask)==id){
			retvalue = canIdAdd(canDev[bus],i);
			if(retvalue != NTCAN_SUCCESS){
#ifndef _WIN32
			  syslog(LOG_ERR, "allowMessage(): canIdAdd() failed with error %d", retvalue);
#endif
			  printf("allowMessage(): canIdAdd() failed with error %ld", retvalue);
			}
		}
	}
}

int initCAN(int bus)
{
    DWORD retvalue;
#ifndef _WIN32
    pthread_mutex_init(&commMutex, NULL);
#endif
    
    retvalue = canOpen(bus, 0, TX_QUEUE_SIZE, RX_QUEUE_SIZE, TX_TIMEOUT, RX_TIMEOUT, &canDev[bus]);
    if(retvalue != NTCAN_SUCCESS){
#ifndef _WIN32
        syslog(LOG_ERR, "initCAN(): canOpen() failed with error %d", retvalue);
#endif
		printf("initCAN(): canOpen() failed with error %ld", retvalue);
        return(1);
    }
    
    retvalue = canSetBaudrate(canDev[bus], 0); // 1 = 1Mbps, 2 = 500kbps, 3 = 250kbps
    if(retvalue != 0)
    {
#ifndef _WIN32
        syslog(LOG_ERR, "initCAN(): canSetBaudrate() failed with error %d", retvalue);
#endif
		printf("initCAN(): canSetBaudrate() failed with error %ld", retvalue);
        return(1);
    }
    
    return(0);
}

int freeCAN(int bus)
{
    canClose(canDev[bus]);
	canDev[bus] = (NTCAN_HANDLE)-1;
	return (0);
}

int canReadMsg(int bus, unsigned long& id, unsigned char& len, unsigned char *data, bool blocking) 
{
    CMSG    msg;
    DWORD   retvalue;
    long    msgCt = 1;
    int     i;
    
    if(blocking){
        retvalue = canRead(canDev[bus], &msg, &msgCt, NULL);
    }else{
        retvalue = canTake(canDev[bus], &msg, &msgCt);
    }
    if(retvalue != NTCAN_SUCCESS){
#ifndef _WIN32
        syslog(LOG_ERR, "canReadMsg(): canRead/canTake error: %ld", retvalue);
#endif
		//printf("canReadMsg(): canRead/canTake error: %ld", retvalue);
        if(retvalue == NTCAN_RX_TIMEOUT)
            return(1);
        else
            return(2);
    }
    if(msgCt == 1){
        id = msg.id;
        len = msg.len;
        for(i = 0; i < msg.len; i++)
            data[i] = msg.data[i];
            
        return(0);
    }
    
    return(1); // No message received, return err
}

int canSendMsg(int bus, unsigned long id, unsigned char len, unsigned char *data, bool blocking) 
{
    CMSG    msg;
    DWORD   retvalue;
    long    msgCt = 1;
    int     i;
    
    msg.id = id;
    msg.len = len & 0x0F;
    for(i = 0; i < len; i++)
        msg.data[i] = data[i];
    
    if(blocking){
        retvalue = canWrite(canDev[bus], &msg, &msgCt, NULL);
    }else{
        retvalue = canSend(canDev[bus], &msg, &msgCt);
    }
    
    if(retvalue != NTCAN_SUCCESS){
#ifndef _WIN32
        syslog(LOG_ERR, "canSendMsg(): canWrite/Send() failed with error %d", retvalue);
#endif
		printf("canSendMsg(): canWrite/Send() failed with error %ld", retvalue);
        return(1);
    }
    return 0;
}


CANAPI_EXTERN_C_END
CANAPI_END
