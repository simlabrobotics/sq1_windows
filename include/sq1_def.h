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

const unsigned char JointNodeID[LEG_COUNT][LEG_JDOF] = {
	{0x01, 0x02, 0x03},
	{0x04, 0x05, 0x06},
	{0x07, 0x08, 0x09},
	{0x0A, 0x0B, 0x0C}
};

class sQ1Leg;
class sQ1Joint;
class sQ1;

class sQ1Joint
{
	int _leg_index;
	int _joint_index;
	bool _enabled;
public:
	sQ1Joint() : _leg_index(-1), _joint_index(-1), _enabled(true) {}
	unsigned char node_id() { return JointNodeID[_leg_index][_joint_index]; }
	int index() { return _joint_index; }
	void index(int lidx, int jidx) { _leg_index = lidx; _joint_index = jidx; }
	bool enabled() { return _enabled; }
	void enabled(bool b) { _enabled = b; }
	void init() {
	}
};

class sQ1Leg
{
	sQ1Joint _joint[LEG_JDOF];
	int _index;
	bool _enabled;
public:
	sQ1Leg() : _index(-1), _enabled(true) {}
	sQ1Joint& joint(int index) { return _joint[index]; }
	const sQ1Joint& joint(int index) const { return _joint[index]; }
	int index() { return _index; }
	void index(int idx) { _index = idx; }
	bool enabled() { return _enabled; }
	void enabled(bool b) { _enabled = b; }
	void init() {
		for (int i=0; i<LEG_JDOF; i++) {
			_joint[i].index(_index, i);
			_joint[i].init();
		}
	}
};

class sQ1
{
	sQ1Leg _leg[LEG_COUNT];
public:
	sQ1Leg& leg(int index) { return _leg[index]; }
	const sQ1Leg& leg(int index) const { return _leg[index]; }
	void init() {
		for (int i=0; i<LEG_COUNT; i++) {
			_leg[i].index(i);
			_leg[i].init();
		}
	}
};

NAMESPACE_SQ1_END

#endif // __SQ1_DEF_H__
