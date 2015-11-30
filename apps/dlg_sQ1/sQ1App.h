#ifndef __sQ1_APP_HEADER__
#define __sQ1_APP_HEADER__

#include <wx/wx.h>

class sQ1App: public wxApp
{
public:
	sQ1App();
	virtual ~sQ1App();

	////////////////////////////////////////////////////////////////////////////
	// properties
	virtual bool OnInit();
	virtual void OnIdle(wxIdleEvent& evt);
	virtual void OnInitCmdLine(wxCmdLineParser &parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser &parser);

	DECLARE_EVENT_TABLE();
	void OnFileNew(wxCommandEvent& event);
	void OnFileOpen(wxCommandEvent& event);
};

DECLARE_APP(sQ1App) // make wxGetApp() available

enum sQ1_COMMAND_IDs {
	sQ1CmdID_COMMAND_BEGIN = 0x0,
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
};

#endif
