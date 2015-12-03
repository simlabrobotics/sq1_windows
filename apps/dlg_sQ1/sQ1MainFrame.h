#ifndef __sQ1_MAINFRAME_HEADER__
#define __sQ1_MAINFRAME_HEADER__

#include <wx/wx.h>

class wxMenuBar;
class wxAuiManager;

#undef MAINFrame_STYLE
#define MAINFrame_STYLE wxCAPTION | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxCLOSE_BOX | wxSTAY_ON_TOP

enum sQ1CmdStatus {
	sQ1_SWITCH_ON_DISABLED,
	sQ1_READY_TO_SWITCH_ON,
	sQ1_SWITCHED_ON,
	sQ1_OPERATION_ENABLED,
	sQ1_OPERATION_ENABLED_HOMED,
};

class MainFrame: public wxFrame
{
	wxMenuBar* _menubar;
	wxAuiManager _frmMgr;

	wxStaticBox *_staticBox;

	wxButton *_btnReadyToSwitchOn;
	wxButton *_btnSwitchOn;
	wxButton *_btnOperationEnable;
	wxButton *_btnShutdown;

	wxButton *_btnHoming;
	wxButton *_btnZeroing;
	wxButton *_btnStandUp;
	wxButton *_btnSitDown;

	wxButton *_btnAbout;
	wxButton *_btnExit;

	sQ1CmdStatus _cmdStatus;
	bool _cmdHomingDoneOnce;
	wxMenu* _menuStates;
	wxMenu* _menuMotion;

public:
	MainFrame(const wxString& title, int width = 800, int height = 600, long style = MAINFrame_STYLE);
	~MainFrame() {
		_frmMgr.UnInit();
	}

	DECLARE_EVENT_TABLE();
	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnReadyToSwitchOn(wxCommandEvent& event);
	void OnUpdateReadyToSwitchOn(wxUpdateUIEvent& event);
	void OnSwitchOn(wxCommandEvent& event);
	void OnUpdateSwitchOn(wxUpdateUIEvent& event);
	void OnOperationEnable(wxCommandEvent& event);
	void OnUpdateOperationEnable(wxUpdateUIEvent& event);
	void OnShutdown(wxCommandEvent& event);
	void OnUpdateShutdown(wxUpdateUIEvent& event);
	void OnHoming(wxCommandEvent& event);
	void OnUpdateHoming(wxUpdateUIEvent& event);
	void OnZero(wxCommandEvent& event);
	void OnUpdateZero(wxUpdateUIEvent& event);
	void OnStandUp(wxCommandEvent& event);
	void OnUpdateStandUp(wxUpdateUIEvent& event);
	void OnSitDown(wxCommandEvent& event);
	void OnUpdateSitDown(wxUpdateUIEvent& event);
	void OnWalkReady(wxCommandEvent& event);
	void OnUpdateWalkReady(wxUpdateUIEvent& event);
	void OnWalk(wxCommandEvent& event);
	void OnUpdateWalk(wxUpdateUIEvent& event);
	void OnWalkStop(wxCommandEvent& event);
	void OnUpdateWalkStop(wxUpdateUIEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnEraseBackground(wxEraseEvent& event);

private:
	void _CreateMenu();
	void _CreateControls();
	void _CreateStatusBar();
	void _UpdateButtonAndMenuStatus();
};

#endif
