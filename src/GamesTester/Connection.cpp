#include "Connection.h"

//(*InternalHeaders(Connection)
#include <wx/string.h>
#include <wx/intl.h>
//*)

#include <wx/utils.h>

//(*IdInit(Connection)
const long Connection::ID_STATICTEXT1 = wxNewId();
const long Connection::ID_TEXTCTRL1 = wxNewId();
const long Connection::ID_BUTTON1 = wxNewId();
const long Connection::ID_BUTTON2 = wxNewId();
const long Connection::ID_STATICTEXT2 = wxNewId();
const long Connection::ID_TEXTCTRL2 = wxNewId();
const long Connection::ID_GAUGE1 = wxNewId();
const long Connection::ID_CHOICE1 = wxNewId();
const long Connection::ID_STATICTEXT3 = wxNewId();
const long Connection::ID_TEXTCTRL3 = wxNewId();
//*)



BEGIN_EVENT_TABLE(Connection,wxDialog)
	//(*EventTable(Connection)
	//*)
END_EVENT_TABLE()

Connection::Connection(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(Connection)
	Create(parent, id, _("Connect to another host"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxSize(299,201));
	Move(wxDefaultPosition);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Hostname :"), wxPoint(24,20), wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	HostnameTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, _("127.0.0.1"), wxPoint(112,16), wxSize(176,23), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	ConnectButton = new wxButton(this, ID_BUTTON1, _("Connect"), wxPoint(24,128), wxSize(264,27), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	ExitButton = new wxButton(this, ID_BUTTON2, _("Exit"), wxPoint(216,160), wxSize(69,27), 0, wxDefaultValidator, _T("ID_BUTTON2"));
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Port :"), wxPoint(24,56), wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	PortTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL2, _("1234"), wxPoint(112,52), wxSize(48,23), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	ConnectionProgressGauge = new wxGauge(this, ID_GAUGE1, 100, wxPoint(24,160), wxSize(184,28), 0, wxDefaultValidator, _T("ID_GAUGE1"));
	ConnectionTypeChoice = new wxChoice(this, ID_CHOICE1, wxPoint(168,52), wxSize(120,25), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	ConnectionTypeChoice->SetSelection( ConnectionTypeChoice->Append(_("Connect to")) );
	ConnectionTypeChoice->Append(_("Host"));
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Nickname :"), wxPoint(24,96), wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	NicknameTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL3, _("UnknownDude"), wxPoint(112,92), wxSize(176,23), 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&Connection::OnConnectButtonClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&Connection::OnExitButtonClick);
	//*)


    NicknameTextCtrl->SetValue(wxGetUserId());

	ExitActivated=0;
}

Connection::~Connection()
{
	//(*Destroy(Connection)
	//*)
}


void Connection::OnConnectButtonClick(wxCommandEvent& event)
{
  Hostname=HostnameTextCtrl->GetValue();
  strncpy(Hostname_cstr, (const char*)HostnameTextCtrl->GetValue().mb_str(wxConvUTF8),128);

  Nickname=NicknameTextCtrl->GetValue();
  wxString curval = PortTextCtrl->GetValue();
  curval.ToLong(&Port);

  if ( ConnectionTypeChoice->GetCurrentSelection()==0 ) { IsHost=0; } else
                                                          { IsHost=1; }

  ExitActivated=0;
  Close();
}

void Connection::OnExitButtonClick(wxCommandEvent& event)
{
  ExitActivated=1;
  Close();
}
