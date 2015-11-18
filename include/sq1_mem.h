#ifndef __SQ1_MEM_H__
#define __SQ1_MEM_H__

#include "sq1_def.h"

NAMESPACE_SQ1_BEGIN

typedef struct tagRobotMemory_sQ1
{
	int enc_actual[LEG_COUNT][LEG_JDOF];
} sQ1_RobotMemory_t;

NAMESPACE_SQ1_END

#endif // __SQ1_MEM_H__
