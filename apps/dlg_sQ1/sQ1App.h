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

#endif
