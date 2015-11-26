/*
 *\brief Constants and enums for CAN communication
 *
 *$Author: Sangyup Yi $
 *$Date: 2015/11/18 19:00:00 $
 *$Revision: 1.0 $
 */ 

#ifndef _CANDEF_H
#define _CANDEF_H


#ifdef __cplusplus
#	define CANAPI_EXTERN_C_BEGIN	extern "C" {
#	define CANAPI_EXTERN_C_END		}
#else
#	define CANAPI_EXTERN_C_BEGIN
#	define CANAPI_EXTERN_C_END
#endif

#ifdef USING_NAMESPACE_CANAPI
#   define CANAPI_BEGIN namespace CANAPI {
#   define CANAPI_END };
#else
#   define CANAPI_BEGIN
#   define CANAPI_END
#endif



CANAPI_BEGIN
CANAPI_EXTERN_C_BEGIN

typedef struct{
	unsigned char	STD_EXT;
	unsigned long	msg_id;         // message identifier
   	unsigned char	data_length;    //
   	char			data[8];        // data array
} can_msg;

#define		STD		(bool)0
#define		EXT		(bool)1


#ifndef LOBYTE
#define MAKEWORD(a, b)      ((unsigned short)(((unsigned char)(((unsigned int)(a)) & 0xff)) | ((unsigned short)((unsigned char)(((unsigned int)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((int)(((unsigned short)(((unsigned int)(a)) & 0xffff)) | ((unsigned int)((unsigned short)(((unsigned int)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((unsigned short)(((unsigned int)(l)) & 0xffff))
#define HIWORD(l)           ((unsigned short)((((unsigned int)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((unsigned char)(((unsigned int)(w)) & 0xff))
#define HIBYTE(w)           ((unsigned char)((((unsigned int)(w)) >> 8) & 0xff))
#endif


////////////////////////////////////////////////

CANAPI_EXTERN_C_END
CANAPI_END

#endif
