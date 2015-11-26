#ifndef __SQ1_DEF_H__
#define __RDEVICEALLEGROHANDCANDEF_H__

#include "sq1_def.h"

NAMESPACE_SQ1_BEGIN

#pragma pack(push)
#pragma pack(1)

////////////////////////////////////////////////////////
// RxPDO (master -> node)
typedef struct tagRxPDO1
{
	union {
		struct {
			unsigned short control_word;
			unsigned char reserved1[6];
		};
		unsigned char data[8];
	};
} tagRxPDO1;

typedef struct tagRxPDO3
{
	union {
		struct {
			long target_position;
			unsigned long profile_velocity;
		};
		unsigned char data[8];
	};
} tagRxPDO1;


////////////////////////////////////////////////////////
// TxPDO (node -> master)
typedef struct tagTxPDO1
{
	union {
		struct {
			unsigned short status_word;
			unsigned char reserved1[6];
		};
		unsigned char data[8];
	};
} tagTxPDO1;

typedef struct tagTxPDO3
{
	union {
		struct {
			long actual_position;
			long digital_input;
		};
		unsigned char data[8];
	};
} tagTxPDO3;

#pragma pack(pop)

NAMESPACE_SQ1_END

#endif // __SQ1_DEF_H__
