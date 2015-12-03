#include <wx/menu.h>
#include <wx/cmdline.h>
#include <wx/treectrl.h>
#include <wx/aui/framemanager.h>

#include "sQ1App.h"
#include "sQ1MainFrame.h"
#include "sQ1Resource.h"
#include "sQ1CANImpl.h"

#include "canopenAPI.h"

USING_NAMESPACE_SQ1

MainFrame::MainFrame(const wxString &title, int width, int height, long style)
: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(width, height), style)
, _menubar(new wxMenuBar())
, _cmdStatus(sQ1_SWITCH_ON_DISABLED)
, _cmdHomingDoneOnce(false)
{
	_frmMgr.SetManagedWindow(this);
	
	_CreateMenu();
	_CreateStatusBar();
	_CreateControls();
	Centre();
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
	EVT_BUTTON(wxID_EXIT, MainFrame::OnQuit)
	EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
	EVT_BUTTON(wxID_ABOUT, MainFrame::OnAbout)
	EVT_MENU(sQ1CmdID_READY_TO_SWITCH_ON, MainFrame::OnReadyToSwitchOn)
	EVT_UPDATE_UI(sQ1CmdID_READY_TO_SWITCH_ON, MainFrame::OnUpdateReadyToSwitchOn)
	EVT_BUTTON(sQ1CmdID_READY_TO_SWITCH_ON, MainFrame::OnReadyToSwitchOn)
	EVT_MENU(sQ1CmdID_SWITCH_ON, MainFrame::OnSwitchOn)
	EVT_UPDATE_UI(sQ1CmdID_SWITCH_ON, MainFrame::OnUpdateSwitchOn)
	EVT_BUTTON(sQ1CmdID_SWITCH_ON, MainFrame::OnSwitchOn)
	EVT_MENU(sQ1CmdID_OPERATION_ENABLE, MainFrame::OnOperationEnable)
	EVT_UPDATE_UI(sQ1CmdID_OPERATION_ENABLE, MainFrame::OnUpdateOperationEnable)
	EVT_BUTTON(sQ1CmdID_OPERATION_ENABLE, MainFrame::OnOperationEnable)
	EVT_MENU(sQ1CmdID_SHUTDOWN, MainFrame::OnShutdown)
	EVT_UPDATE_UI(sQ1CmdID_SHUTDOWN, MainFrame::OnUpdateShutdown)
	EVT_BUTTON(sQ1CmdID_SHUTDOWN, MainFrame::OnShutdown)
	EVT_MENU(sQ1CmdID_HOMING, MainFrame::OnHoming)
	EVT_UPDATE_UI(sQ1CmdID_HOMING, MainFrame::OnUpdateHoming)
	EVT_BUTTON(sQ1CmdID_HOMING, MainFrame::OnHoming)
	EVT_MENU(sQ1CmdID_ZERO, MainFrame::OnZero)
	EVT_UPDATE_UI(sQ1CmdID_ZERO, MainFrame::OnUpdateZero)
	EVT_BUTTON(sQ1CmdID_ZERO, MainFrame::OnZero)
	EVT_MENU(sQ1CmdID_STANDUP, MainFrame::OnStandUp)
	EVT_UPDATE_UI(sQ1CmdID_STANDUP, MainFrame::OnUpdateStandUp)
	EVT_BUTTON(sQ1CmdID_STANDUP, MainFrame::OnStandUp)
	EVT_MENU(sQ1CmdID_SITDOWN, MainFrame::OnSitDown)
	EVT_UPDATE_UI(sQ1CmdID_SITDOWN, MainFrame::OnUpdateSitDown)
	EVT_BUTTON(sQ1CmdID_SITDOWN, MainFrame::OnSitDown)
	EVT_MENU(sQ1CmdID_WALK_READY, MainFrame::OnWalkReady)
	EVT_UPDATE_UI(sQ1CmdID_WALK_READY, MainFrame::OnUpdateWalkReady)
	EVT_BUTTON(sQ1CmdID_WALK_READY, MainFrame::OnWalkReady)
	EVT_MENU(sQ1CmdID_WALK, MainFrame::OnWalk)
	EVT_UPDATE_UI(sQ1CmdID_WALK, MainFrame::OnUpdateWalk)
	EVT_BUTTON(sQ1CmdID_WALK, MainFrame::OnWalk)
	EVT_MENU(sQ1CmdID_WALK_STOP, MainFrame::OnWalkStop)
	EVT_UPDATE_UI(sQ1CmdID_WALK_STOP, MainFrame::OnUpdateWalkStop)
	EVT_BUTTON(sQ1CmdID_WALK_STOP, MainFrame::OnWalkStop)
	EVT_ERASE_BACKGROUND(MainFrame::OnEraseBackground)
END_EVENT_TABLE()


void MainFrame::OnQuit(wxCommandEvent &event)
{
	Close(true);
}

void MainFrame::OnAbout(wxCommandEvent &event)
{
	wxMessageBox("sQ1 (SimLab Quadruped 1) Control Panel v1.0\nCopyright (c) SimLab",
        "About...", wxOK | wxICON_INFORMATION, this);

	/*wxDialog *dlg = new wxDialog(this, wxID_ANY, wxT("About..."));
	dlg->ShowModal();
	dlg->Destroy();*/
}

void MainFrame::OnClose(wxCloseEvent& event)
{
}

void MainFrame::OnEraseBackground(wxEraseEvent& event)
{
	SetBackgroundColour(wxColor(236,233,216));
	event.Skip();
}

void MainFrame::OnReadyToSwitchOn(wxCommandEvent& event)
{
	sq1::ReadyToSwitchOn();
	_cmdStatus = sQ1_READY_TO_SWITCH_ON;
	SetStatusText("Ready To Switch On");
}

void MainFrame::OnUpdateReadyToSwitchOn(wxUpdateUIEvent& event)
{
	if (_cmdStatus != sQ1_SWITCH_ON_DISABLED)
		event.Enable(false);
	else
		event.Enable(true);
}

void MainFrame::OnSwitchOn(wxCommandEvent& event)
{
	sq1::SwitchedOn();
	_cmdStatus = sQ1_SWITCHED_ON;
	SetStatusText("Switched On");
}

void MainFrame::OnUpdateSwitchOn(wxUpdateUIEvent& event)
{
	if (_cmdStatus != sQ1_READY_TO_SWITCH_ON)
		event.Enable(false);
	else
		event.Enable(true);
}

void MainFrame::OnOperationEnable(wxCommandEvent& event)
{
	sq1::OperationEnable();
	if (_cmdHomingDoneOnce) {
		_cmdStatus = sQ1_OPERATION_ENABLED_HOMED;
		SetStatusText("Operation Enabled (Homing is done)");
	}
	else {
		_cmdStatus = sQ1_OPERATION_ENABLED;
		SetStatusText("Operation Enabled (Homing is required)");
	}
}

void MainFrame::OnUpdateOperationEnable(wxUpdateUIEvent& event)
{
	if (_cmdStatus != sQ1_SWITCHED_ON)
		event.Enable(false);
	else
		event.Enable(true);
}

void MainFrame::OnShutdown(wxCommandEvent& event)
{
	sq1::Shutdown();
	_cmdStatus = sQ1_SWITCH_ON_DISABLED;
	SetStatusText("Switch On Disabled");
}

void MainFrame::OnUpdateShutdown(wxUpdateUIEvent& event)
{
	if (_cmdStatus < sQ1_OPERATION_ENABLED)
		event.Enable(false);
	else
		event.Enable(true);
}

void MainFrame::OnHoming(wxCommandEvent& event)
{
	sq1::StartHoming();
	_cmdStatus = sQ1_OPERATION_ENABLED_HOMED;
	SetStatusText("Operation Enabled (Homing is done)");
	_cmdHomingDoneOnce = true;
}

void MainFrame::OnUpdateHoming(wxUpdateUIEvent& event)
{
	if (_cmdStatus != sQ1_OPERATION_ENABLED)
		event.Enable(false);
	else
		event.Enable(true);
}

void MainFrame::OnZero(wxCommandEvent& event)
{
	sq1::MotionZero();
}

void MainFrame::OnUpdateZero(wxUpdateUIEvent& event)
{
	if (_cmdStatus < sQ1_OPERATION_ENABLED_HOMED ||
		sq1::GetHomingDone() != sq1::HOMING_DONE)
		event.Enable(false);
	else
		event.Enable(true);
}

void MainFrame::OnStandUp(wxCommandEvent& event)
{
	sq1::MotionStretch();
}

void MainFrame::OnUpdateStandUp(wxUpdateUIEvent& event)
{
	if (_cmdStatus < sQ1_OPERATION_ENABLED_HOMED ||
		sq1::GetHomingDone() != sq1::HOMING_DONE)
		event.Enable(false);
	else
		event.Enable(true);
}

void MainFrame::OnSitDown(wxCommandEvent& event)
{
	sq1::MotionSquat();
}

void MainFrame::OnUpdateSitDown(wxUpdateUIEvent& event)
{
	if (_cmdStatus < sQ1_OPERATION_ENABLED_HOMED ||
		sq1::GetHomingDone() != sq1::HOMING_DONE)
		event.Enable(false);
	else
		event.Enable(true);
}

void MainFrame::OnWalkReady(wxCommandEvent& event)
{
	sq1::MotionWalkReady();
}

void MainFrame::OnUpdateWalkReady(wxUpdateUIEvent& event)
{
//	if (_cmdStatus < sQ1_OPERATION_ENABLED_HOMED ||
//		sq1::GetHomingDone() != sq1::HOMING_DONE)
		event.Enable(false);
//	else
//		event.Enable(true);
}

void MainFrame::OnWalk(wxCommandEvent& event)
{
	sq1::MotionWalk();
}

void MainFrame::OnUpdateWalk(wxUpdateUIEvent& event)
{
//	if (_cmdStatus < sQ1_OPERATION_ENABLED_HOMED ||
//		sq1::GetHomingDone() != sq1::HOMING_DONE)
		event.Enable(false);
//	else
//		event.Enable(true);
}

void MainFrame::OnWalkStop(wxCommandEvent& event)
{
	//sq1::MotionWalkReady();
}

void MainFrame::OnUpdateWalkStop(wxUpdateUIEvent& event)
{
//	if (_cmdStatus < sQ1_OPERATION_ENABLED_HOMED ||
//		sq1::GetHomingDone() != sq1::HOMING_DONE)
		event.Enable(false);
//	else
//		event.Enable(true);
}

void MainFrame::_CreateMenu()
{
	/////////////////////////////////////////////////////////////////////////
	// 아이콘 설정
	wxIcon icon(wxT("res/app.ico"), wxBITMAP_TYPE_ICO);
	SetIcon(icon);

	/////////////////////////////////////////////////////////////////////////
	// Menu
	wxMenuItem* pMenuItem = 0x0;

	// File:
	wxMenu* pMenu = new wxMenu();
	pMenu->Append(wxID_EXIT, wxT("&Quit"));
	_menubar->Append(pMenu, wxT("&File"));

	// States:
	pMenu = new wxMenu();
	_menuStates = pMenu;

	pMenuItem = pMenu->Append(sQ1CmdID_READY_TO_SWITCH_ON, wxT("Ready To Switch On"));
	pMenuItem = pMenu->Append(sQ1CmdID_SWITCH_ON, wxT("Switch On"));
	pMenuItem->Enable(false);
	pMenuItem = pMenu->Append(sQ1CmdID_OPERATION_ENABLE, wxT("Operation Enable"));
	pMenuItem->Enable(false);
	pMenuItem = pMenu->Append(sQ1CmdID_SHUTDOWN, wxT("Shutdown"));
	pMenuItem->Enable(false);

	_menubar->Append(pMenu, wxT("&States"));

	// Motion:
	pMenu = new wxMenu();
	_menuMotion = pMenu;

	pMenuItem = pMenu->Append(sQ1CmdID_HOMING, wxT("Start Homing"));
	pMenuItem->Enable(false);
	pMenuItem = pMenu->Append(sQ1CmdID_ZERO, wxT("Go to Zero Position"));
	pMenuItem->Enable(false);

	pMenuItem = pMenu->AppendSeparator();
	pMenuItem = pMenu->Append(sQ1CmdID_STANDUP, wxT("Stand Up"));
	pMenuItem->Enable(false);
	pMenuItem = pMenu->Append(sQ1CmdID_SITDOWN, wxT("Sit Down"));
	pMenuItem->Enable(false);

	pMenuItem = pMenu->AppendSeparator();
	pMenuItem = pMenu->Append(sQ1CmdID_WALK_READY, wxT("Ready To Walk"));
	pMenuItem->Enable(false);
	pMenuItem = pMenu->Append(sQ1CmdID_WALK, wxT("Walk"));
	pMenuItem->Enable(false);
	pMenuItem = pMenu->Append(sQ1CmdID_WALK_STOP, wxT("Stop Walking"));
	pMenuItem->Enable(false);

	_menubar->Append(pMenu, wxT("&Motion"));

	// Help:
	pMenu = new wxMenu();
	pMenu->Append(wxID_ABOUT, wxT("&About..."));
	_menubar->Append(pMenu, wxT("&Help"));

	SetMenuBar(_menubar);
}

void MainFrame::_CreateControls()
{
	SetBackgroundColour(wxColor(236,233,216));

	//_staticBox = new wxStaticBox(this, sQ1CmdID_DUMMY, wxT(""), wxPoint(8,5), wxSize(260,215));

	_btnReadyToSwitchOn = new wxButton(this, sQ1CmdID_READY_TO_SWITCH_ON, wxT("Ready To Switch On"), wxPoint(10, 5), wxSize(250, 32), 0, wxDefaultValidator, wxT("sQ1CmdID_READY_TO_SWITCH_ON"));
	_btnReadyToSwitchOn->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));
	_btnSwitchOn = new wxButton(this, sQ1CmdID_SWITCH_ON, wxT("Switch On"), wxPoint(10, 40), wxSize(250, 32), 0, wxDefaultValidator, wxT("sQ1CmdID_SWITCH_ON"));
	_btnSwitchOn->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));
	_btnOperationEnable = new wxButton(this, sQ1CmdID_OPERATION_ENABLE, wxT("Operation Enable"), wxPoint(10, 75), wxSize(250, 32), 0, wxDefaultValidator, wxT("sQ1CmdID_OPERATION_ENABLE"));
	_btnOperationEnable->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));
	_btnShutdown = new wxButton(this, sQ1CmdID_SHUTDOWN, wxT("Shutdown"), wxPoint(10, 110), wxSize(250, 32), 0, wxDefaultValidator, wxT("sQ1CmdID_SHUTDOWN"));
	_btnShutdown->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));

	_btnHoming = new wxButton(this, sQ1CmdID_HOMING, wxT("Homing"), wxPoint(10, 175), wxSize(250, 32), 0, wxDefaultValidator, wxT("sQ1CmdID_HOMING"));
	_btnHoming->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));
	_btnZeroing = new wxButton(this, sQ1CmdID_ZERO, wxT("Goto Zero"), wxPoint(10, 210), wxSize(250, 32), 0, wxDefaultValidator, wxT("sQ1CmdID_ZERO"));
	_btnZeroing->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));

	_btnStandUp = new wxButton(this, sQ1CmdID_STANDUP, wxT("Stand Up"), wxPoint(10, 275), wxSize(250, 32), 0, wxDefaultValidator, wxT("sQ1CmdID_STANDUP"));
	_btnStandUp->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));
	_btnSitDown = new wxButton(this, sQ1CmdID_SITDOWN, wxT("Sit Down"), wxPoint(10, 310), wxSize(250, 32), 0, wxDefaultValidator, wxT("sQ1CmdID_SITDOWN"));
	_btnSitDown->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));

	_btnAbout = new wxButton(this, wxID_ABOUT, wxT("About"), wxPoint(10, 375), wxSize(250, 32), 0, wxDefaultValidator, wxT("wxID_ABOUT"));
	_btnAbout->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));
	_btnExit = new wxButton(this, wxID_EXIT, wxT("Exit"), wxPoint(10, 410), wxSize(250, 32), 0, wxDefaultValidator, wxT("wxID_EXIT"));
	_btnExit->SetFont(wxFont(14, wxSWISS, wxNORMAL, wxBOLD, FALSE, wxT("Arial")));
	_btnExit->SetDefault();
}

void MainFrame::_CreateStatusBar()
{
	CreateStatusBar();
	SetStatusText("Switch On Disabled");
}

void MainFrame::_UpdateButtonAndMenuStatus()
{
	if (_cmdStatus >= sQ1_OPERATION_ENABLED_HOMED) {
	}
	else if (_cmdStatus >= sQ1_OPERATION_ENABLED) {
	}
	else if (_cmdStatus >= sQ1_SWITCHED_ON) {
	}
	else if (_cmdStatus >= sQ1_READY_TO_SWITCH_ON) {
	}
}
