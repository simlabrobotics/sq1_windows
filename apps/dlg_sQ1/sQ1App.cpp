
#include <wx/menu.h>
#include <wx/cmdline.h>
#include <wx/treectrl.h>
#include <wx/aui/framemanager.h>

#include "sQ1App.h"
#include "sQ1CANImpl.h"

#include "canopenAPI.h"

USING_NAMESPACE_SQ1

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
     { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters",
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	 /*{ wxCMD_LINE_OPTION, "port", "port", "port number", 
		wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL},*/
     { wxCMD_LINE_NONE }
};

class MainFrame: public wxFrame
{
	wxMenuBar* _menubar;
	wxAuiManager _frmMgr;
public:
	MainFrame(const wxString& title, int width = 800, int height = 600);
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

	// CAN communication
};

MainFrame::MainFrame(const wxString &title, int width, int height)
: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(width, height))
, _menubar(new wxMenuBar())
{
	_frmMgr.SetManagedWindow(this);

	/////////////////////////////////////////////////////////////////////////////
	////// 아이콘 설정
	///wxIcon icon(wxT("res/app.ico"), wxBITMAP_TYPE_ICO);
	///SetIcon(icon);

	/////////////////////////////////////////////////////////////////////////
	// Menu
	wxMenuItem* pMenuItem = 0x0;

	// File:
	wxMenu* pMenu = new wxMenu();
	pMenu->Append(wxID_EXIT, wxT("&Quit"));
	_menubar->Append(pMenu, wxT("&File"));

	// States:
	pMenu = new wxMenu();
	
	pMenuItem = pMenu->Append(sQ1CmdID_READY_TO_SWITCH_ON, wxT("Ready To Switch On"));
	pMenuItem = pMenu->Append(sQ1CmdID_SWITCH_ON, wxT("Switch On"));
	pMenuItem = pMenu->Append(sQ1CmdID_OPERATION_ENABLE, wxT("Operation Enable"));
	pMenuItem = pMenu->Append(sQ1CmdID_SHUTDOWN, wxT("Shutdown"));

	_menubar->Append(pMenu, wxT("&States"));

	// Motion:
	pMenu = new wxMenu();

	pMenuItem = pMenu->Append(sQ1CmdID_HOMING, wxT("Start Homing"));
	pMenuItem = pMenu->Append(sQ1CmdID_ZERO, wxT("Go to Zero Position"));

	pMenuItem = pMenu->AppendSeparator();
	pMenuItem = pMenu->Append(sQ1CmdID_STANDUP, wxT("Stand Up"));
	pMenuItem = pMenu->Append(sQ1CmdID_SITDOWN, wxT("Sit Down"));

	pMenuItem = pMenu->AppendSeparator();
	pMenuItem = pMenu->Append(sQ1CmdID_WALK_READY, wxT("Ready To Walk"));
	pMenuItem = pMenu->Append(sQ1CmdID_WALK, wxT("Walk"));
	pMenuItem = pMenu->Append(sQ1CmdID_WALK_STOP, wxT("Stop Walking"));

	_menubar->Append(pMenu, wxT("&Motion"));

	// Help:
	pMenu = new wxMenu();
	pMenu->Append(wxID_ABOUT, wxT("&About..."));
	_menubar->Append(pMenu, wxT("&Help"));

	SetMenuBar(_menubar);
	
	Centre();
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
	EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
	EVT_MENU(sQ1CmdID_READY_TO_SWITCH_ON, MainFrame::OnReadyToSwitchOn)
	EVT_UPDATE_UI(sQ1CmdID_READY_TO_SWITCH_ON, MainFrame::OnUpdateReadyToSwitchOn)
	EVT_MENU(sQ1CmdID_SWITCH_ON, MainFrame::OnSwitchOn)
	EVT_UPDATE_UI(sQ1CmdID_SWITCH_ON, MainFrame::OnUpdateSwitchOn)
	EVT_MENU(sQ1CmdID_OPERATION_ENABLE, MainFrame::OnOperationEnable)
	EVT_UPDATE_UI(sQ1CmdID_OPERATION_ENABLE, MainFrame::OnUpdateOperationEnable)
	EVT_MENU(sQ1CmdID_SHUTDOWN, MainFrame::OnShutdown)
	EVT_UPDATE_UI(sQ1CmdID_SHUTDOWN, MainFrame::OnUpdateShutdown)
	EVT_MENU(sQ1CmdID_HOMING, MainFrame::OnHoming)
	EVT_UPDATE_UI(sQ1CmdID_HOMING, MainFrame::OnUpdateHoming)
	EVT_MENU(sQ1CmdID_ZERO, MainFrame::OnZero)
	EVT_UPDATE_UI(sQ1CmdID_ZERO, MainFrame::OnUpdateZero)
	EVT_MENU(sQ1CmdID_STANDUP, MainFrame::OnStandUp)
	EVT_UPDATE_UI(sQ1CmdID_STANDUP, MainFrame::OnUpdateStandUp)
	EVT_MENU(sQ1CmdID_SITDOWN, MainFrame::OnSitDown)
	EVT_UPDATE_UI(sQ1CmdID_SITDOWN, MainFrame::OnUpdateSitDown)
	EVT_MENU(sQ1CmdID_WALK_READY, MainFrame::OnWalkReady)
	EVT_UPDATE_UI(sQ1CmdID_WALK_READY, MainFrame::OnUpdateWalkReady)
	EVT_MENU(sQ1CmdID_WALK, MainFrame::OnWalk)
	EVT_UPDATE_UI(sQ1CmdID_WALK, MainFrame::OnUpdateWalk)
	EVT_MENU(sQ1CmdID_WALK_STOP, MainFrame::OnWalkStop)
	EVT_UPDATE_UI(sQ1CmdID_WALK_STOP, MainFrame::OnUpdateWalkStop)
END_EVENT_TABLE()


void MainFrame::OnQuit(wxCommandEvent &event)
{
	Close(true);
}

void MainFrame::OnAbout(wxCommandEvent &event)
{
	wxDialog *dlg = new wxDialog(this, wxID_ANY, wxT("About..."));
	dlg->ShowModal();
	dlg->Destroy();
}

void MainFrame::OnReadyToSwitchOn(wxCommandEvent& event)
{
	sq1::ReadyToSwitchOn();
}

void MainFrame::OnUpdateReadyToSwitchOn(wxUpdateUIEvent& event)
{
}

void MainFrame::OnSwitchOn(wxCommandEvent& event)
{
	sq1::SwitchedOn();
}

void MainFrame::OnUpdateSwitchOn(wxUpdateUIEvent& event)
{
}

void MainFrame::OnOperationEnable(wxCommandEvent& event)
{
	sq1::OperationEnable();
}

void MainFrame::OnUpdateOperationEnable(wxUpdateUIEvent& event)
{
}

void MainFrame::OnShutdown(wxCommandEvent& event)
{
	sq1::Shutdown();
}

void MainFrame::OnUpdateShutdown(wxUpdateUIEvent& event)
{
}

void MainFrame::OnHoming(wxCommandEvent& event)
{
	sq1::StartHoming();
}

void MainFrame::OnUpdateHoming(wxUpdateUIEvent& event)
{
}

void MainFrame::OnZero(wxCommandEvent& event)
{
	sq1::MotionZero();
}

void MainFrame::OnUpdateZero(wxUpdateUIEvent& event)
{
	if (sq1::GetHomingDone() != sq1::HOMING_DONE)
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
	if (sq1::GetHomingDone() != sq1::HOMING_DONE)
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
	if (sq1::GetHomingDone() != sq1::HOMING_DONE)
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
//	if (sq1::GetHomingDone() != sq1::HOMING_DONE)
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
//	if (sq1::GetHomingDone() != sq1::HOMING_DONE)
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
//	if (sq1::GetHomingDone() != sq1::HOMING_DONE)
		event.Enable(false);
//	else
//		event.Enable(true);
}

/////////////////////////////////////////////////////////////////////////////
//

IMPLEMENT_APP(sQ1App)

BEGIN_EVENT_TABLE(sQ1App, wxApp)
	EVT_MENU(wxID_NEW, sQ1App::OnFileNew)
	EVT_MENU(wxID_OPEN, sQ1App::OnFileOpen)
END_EVENT_TABLE()

sQ1App::sQ1App()
: wxApp() 
{
}

sQ1App::~sQ1App() 
{
	sq1::DriveOff();
	sq1::CloseCAN();
}

bool sQ1App::OnInit()
{
	sq1::InitVariables();
	// open CAN channel:
	if (!sq1::OpenCAN())
		return false;
	sq1::DriveReset();
	sq1::DriveInit();

	

	wxInitAllImageHandlers();

	int width = 800;
	int height = 600;
	MainFrame *pFrame = new MainFrame(wxT("sQ1 Control Panel"));
	
	wxCmdLineParser parser(argc, argv);
	OnInitCmdLine(parser);
	parser.Parse();
	OnCmdLineParsed(parser);
	
	pFrame->Show(true);
	SetTopWindow(pFrame);
	
	return true;
};

void sQ1App::OnIdle(wxIdleEvent& evt)
{
	static int sync_counter = 0;


	for (int ch = 0; ch < (int)CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		ProcessCANMessage(ch);
	}
	//Sleep(5);
	sync_counter++;
	if (sync_counter == 100) {
				
		for (int ch = 0; ch < (int)CAN_Ch_COUNT; ch++)
		{
			if (!CAN_Ch_Enabled[ch]) continue;
			for (int node = 0; node < (int)NODE_COUNT; node++)
			{
				if (!NODE_Enabled[ch][node]) continue;
				can_pdo_rx1(CAN_Ch[ch], JointNodeID[ch][node], targetPosition[ch][node], targetVelocity[ch][node]);
				can_pdo_rx3(CAN_Ch[ch], JointNodeID[ch][node], controlWord[ch][node], modeOfOperation);

				if (GetHomingDone() == HOMING_DONE &&
					(controlWord[ch][node]&0x0010) != 0) { // when "Set new point" bit is set...
					controlWord[ch][node] &= 0xFF8F; // masking irrelevant bits
					controlWord[ch][node] |= 0x00; // clear all operation mode specific bits
				}
			}
		}
				
		for (int ch = 0; ch < (int)CAN_Ch_COUNT; ch++)
		{
			if (!CAN_Ch_Enabled[ch]) continue;
			can_sync(CAN_Ch[ch]);
		}

		sync_counter = 0;
	}

	evt.RequestMore(); // render continuously, not only once on idle
}

void sQ1App::OnFileNew(wxCommandEvent &event)
{
}

void sQ1App::OnFileOpen(wxCommandEvent &event)
{
	wxFileDialog* dlg = new wxFileDialog(NULL);

	if (wxID_OK != dlg->ShowModal() ) {
		dlg->Destroy();
		return;
	}

	wxString path = dlg->GetPath();

	dlg->Destroy();
}

void sQ1App::OnInitCmdLine(wxCmdLineParser &parser)
{
	parser.SetDesc(g_cmdLineDesc);
	parser.SetSwitchChars(wxT("-"));
}

bool sQ1App::OnCmdLineParsed(wxCmdLineParser &parser)
{
	//long port = 5150;
	//if (parser.Found(wxT("port"), &port)) {
	//}

	return true;
}
