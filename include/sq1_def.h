#ifndef __SQ1_DEF_H__
#define __SQ1_DEF_H__

#ifdef NOT_USING_NAMESPACE_SQ1
#define NAMESPACE_SQ1_BEGIN
#define NAMESPACE_SQ1_END
#define USING_NAMESPACE_SQ1
#else
#define NAMESPACE_SQ1_BEGIN		namespace sq1 {
#define NAMESPACE_SQ1_END		}
#define USING_NAMESPACE_SQ1		using namespace sq1;
#endif

NAMESPACE_SQ1_BEGIN

enum LegIndex
{
	FL = 0x00,
	FR,
	RL,
	RR,
	LEG_COUNT
};

enum JointIndex
{
	SH_ROLL,
	SH_PITCH,
	KNEE,
	LEG_JDOF
};

const int JDOF = LEG_COUNT*LEG_JDOF; //12;

NAMESPACE_SQ1_END

#endif // __SQ1_DEF_H__
