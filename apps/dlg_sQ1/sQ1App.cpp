
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

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
     { wxCMD_LINE_SWITCH, "h", "help", "displays help on the command line parameters",
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	 /*{ wxCMD_LINE_OPTION, "port", "port", "port number", 
		wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL},*/
     { wxCMD_LINE_NONE }
};

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

	int width = 285;
	int height = 540;
	MainFrame *pFrame = new MainFrame(wxT("sQ1 Control Panel"), width, height);
	
	wxCmdLineParser parser(argc, argv);
	OnInitCmdLine(parser);
	parser.Parse();
	OnCmdLineParsed(parser);
	
	pFrame->Show(true);
	SetTopWindow(pFrame);
	
	Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(sQ1App::OnIdle) );

	return true;
};

void sQ1App::OnIdle(wxIdleEvent& evt)
{
	static int sync_counter = 0;

	sync_counter++;

	if (sync_counter == 1) {
				
		for (int ch = 0; ch < (int)CAN_Ch_COUNT; ch++)
		{
			if (!CAN_Ch_Enabled[ch]) continue;

			for (int node = 0; node < (int)NODE_COUNT; node++)
			{
				if (!NODE_Enabled[ch][node]) continue;

				if (GetHomingDone() == HOMING_DONE &&
					(statusWord[ch][node]&0x1000) != 0 &&
					(controlWord[ch][node]&0x0010) != 0) { // when "Set new point" bit is set...
					controlWord[ch][node] &= 0xDF8F; // masking irrelevant bits
					controlWord[ch][node] |= 0x0000; // clear all operation mode specific bits
				}

				can_pdo_rx1(CAN_Ch[ch], JointNodeID[ch][node], targetPosition[ch][node], targetVelocity[ch][node]);
				can_pdo_rx3(CAN_Ch[ch], JointNodeID[ch][node], controlWord[ch][node], modeOfOperation);
			}
		}
				
		for (int ch = 0; ch < (int)CAN_Ch_COUNT; ch++)
		{
			if (!CAN_Ch_Enabled[ch]) continue;
			can_sync(CAN_Ch[ch]);
		}

		sync_counter = 0;
	}

	Sleep(5);

	for (int ch = 0; ch < (int)CAN_Ch_COUNT; ch++)
	{
		if (!CAN_Ch_Enabled[ch]) continue;
		ProcessCANMessage(ch);
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
