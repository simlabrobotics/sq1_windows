#ifndef __sQ1_RESOURCE_HEADER__
#define __sQ1_RESOURCE_HEADER__

#include "wx/defs.h"

enum sQ1_COMMAND_IDs {
	sQ1CmdID_COMMAND_BEGIN = wxID_HIGHEST + 1,
	sQ1CmdID_READY_TO_SWITCH_ON,
	sQ1CmdID_SWITCH_ON, 
	sQ1CmdID_OPERATION_ENABLE,
	sQ1CmdID_SHUTDOWN,
	sQ1CmdID_HOMING,
	sQ1CmdID_ZERO,
	sQ1CmdID_STANDUP,
	sQ1CmdID_SITDOWN,
	sQ1CmdID_WALK_READY,
	sQ1CmdID_WALK,
	sQ1CmdID_WALK_STOP,
	sQ1CmdID_COMMAND_END,
	sQ1CmdID_DUMMY,
};

#endif