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
#include "Kvaser/canlib.h"

CANAPI_BEGIN
CANAPI_EXTERN_C_BEGIN

#define CH_COUNT			(int)4 // number of CAN channels

static int hCAN[CH_COUNT] = {-1, -1, -1, -1}; // CAN channel handles

int initCAN(int bus)
{
	assert(bus >= 0 && bus < CH_COUNT);

	canStatus ret;
	
	printf("<< CAN: Initialize Library...\n");
	canInitializeLibrary();
	printf("\t- Done\n");
	Sleep(200);

	printf("<< CAN: Open Channel...\n");
	hCAN[bus] = canOpenChannel(bus, canOPEN_EXCLUSIVE);
	if (hCAN[bus] < 0) return -1;
	printf("\t- Ch.%2d (OK)\n", bus);
	printf("\t- Done\n");
	Sleep(200);

	printf("<< CAN: Set Bus Parameter...\n");
	ret = canSetBusParams(hCAN[bus], BAUD_1M, 0, 0, 0, 0, 0);
	if (ret < 0) return -2;
	printf("\t- Done\n");
	Sleep(200);

	printf("<< CAN: Bus On...\n");
	ret = canBusOn(hCAN[bus]);
	if (ret < 0) return -3;
	printf("\t- Done\n");
	Sleep(200);

	return 0;
}

int resetCAN(int bus)
{
	assert(bus >= 0 && bus < CH_COUNT);

	canStatus ret;

	printf("<< CAN: Reset Bus...\n");
	ret = canResetBus(hCAN[bus]);
	if (ret < 0) return ret;
	printf("\t- Done\n");
	Sleep(200);

	return 0;
}

int freeCAN(int bus)
{
	assert(bus >= 0 && bus < CH_COUNT);

	canStatus ret;

	printf("<< CAN: Close...\n");
	ret = canClose(hCAN[bus]);
	if (ret < 0) return ret;
	hCAN[bus] = 0;
	printf("\t- Done\n");
	Sleep(200);
	return 0;
}

int canReadMsg(int bus, unsigned long& id, unsigned char& len, unsigned char *data, bool /*blocking*/) 
{
	long Rxid;
	unsigned char rdata[8];
	unsigned int dlc;
	unsigned int flag;
	unsigned long time;
	canStatus ret;

	memset(rdata, NULL, sizeof(rdata));
	ret = canRead(hCAN[bus], &Rxid, rdata, &dlc, &flag, &time);
	if (ret != canOK) return ret;

	id = (unsigned long)Rxid;
	len = (unsigned char)dlc;
	for(int nd=0; nd<(int)dlc; nd++) data[nd] = rdata[nd];

#ifdef CAN_PRINT_Rx_MESSAGE
	printf("    Rx %04xh (len=%d)", id, len);
	for(int nd=0; nd<len; nd++) printf(" %02X ", data[nd]);
	printf("\n");
#endif

	return 0;
}

int canSendMsg(int bus, unsigned long id, unsigned char len, unsigned char *data, bool /*blocking*/) 
{
#ifdef CAN_PRINT_Tx_MESSAGE
	printf("    Tx %04xh (len=%d)", id, len);
	for(int nd=0; nd<len; nd++) printf(" %02X ", data[nd]);
	printf("\n");
#endif

	canStatus ret = canWrite(hCAN[bus], (long)id, data, (unsigned int)len, STD);
	if (ret != canOK)
	{
		return ret;
	}

	return 0;
}

CANAPI_EXTERN_C_END
CANAPI_END
