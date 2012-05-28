/***************************************************************
 * Name:      GamesTesterMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    Ammar Qammaz (ammarkov@gmail.com)
 * Created:   2012-05-28
 * Copyright: Ammar Qammaz (http://ammar.gr)
 * License:
 **************************************************************/

#include "GamesTesterMain.h"
#include <wx/msgdlg.h>
#include "../RemoteVariableSupport.h"
#include "Connection.h"

#include <wx/image.h>
#include <wx/dc.h>
#include <wx/dcclient.h>
//(*InternalHeaders(GamesTesterFrame)
#include <wx/string.h>
#include <wx/intl.h>
//*)

wxImage OurMark;
wxImage OpponentMark;


struct VariableShare * vsh = 0;

wxString OurName;
wxString OpponentName;

volatile unsigned int OurMove=0;
volatile unsigned int OpponentMove=0;
char OurMessage[128]={0};
char OpponentMessage[128]={0};




//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

//(*IdInit(GamesTesterFrame)
const long GamesTesterFrame::ID_TEXTCTRL1 = wxNewId();
const long GamesTesterFrame::ID_STATICTEXT1 = wxNewId();
const long GamesTesterFrame::ID_BUTTON1 = wxNewId();
const long GamesTesterFrame::ID_TEXTCTRL2 = wxNewId();
const long GamesTesterFrame::ID_STATICBOX1 = wxNewId();
const long GamesTesterFrame::ID_BUTTON2 = wxNewId();
const long GamesTesterFrame::idMenuQuit = wxNewId();
const long GamesTesterFrame::idMenuAbout = wxNewId();
const long GamesTesterFrame::ID_STATUSBAR1 = wxNewId();
const long GamesTesterFrame::ID_TIMER1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(GamesTesterFrame,wxFrame)
    //(*EventTable(GamesTesterFrame)
    //*)
   EVT_PAINT(GamesTesterFrame::OnPaint)
END_EVENT_TABLE()

GamesTesterFrame::GamesTesterFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(GamesTesterFrame)
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;

    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    SetClientSize(wxSize(870,562));
    OurTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxPoint(600,448), wxSize(256,24), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Label"), wxPoint(784,416), wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    SendButton = new wxButton(this, ID_BUTTON1, _("Send"), wxPoint(752,480), wxSize(104,27), 0, wxDefaultValidator, _T("ID_BUTTON1"));
    ChatTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxPoint(600,18), wxSize(256,424), wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    StaticBox1 = new wxStaticBox(this, ID_STATICBOX1, _("Game area :P"), wxPoint(16,8), wxSize(568,496), 0, _T("ID_STATICBOX1"));
    NudgeButton = new wxButton(this, ID_BUTTON2, _("Nudge"), wxPoint(600,488), wxSize(56,19), 0, wxDefaultValidator, _T("ID_BUTTON2"));
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[1] = { -1 };
    int __wxStatusBarStyles_1[1] = { wxSB_NORMAL };
    StatusBar1->SetFieldsCount(1,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(1,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    ClockTimer.SetOwner(this, ID_TIMER1);
    ClockTimer.Start(100, false);

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GamesTesterFrame::OnSendButtonClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GamesTesterFrame::OnNudgeButtonClick);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&GamesTesterFrame::OnQuit);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&GamesTesterFrame::OnAbout);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&GamesTesterFrame::OnClockTimerTrigger);
    //*)

   OurMark.LoadFile(wxT("src/GamesTester/images/ourmark.png"));
   OpponentMark.LoadFile(wxT("src/GamesTester/images/oppmark.png"));

   OpponentName = wxT("Rival");

   Connection new_con_window(0);
   char hostname[128]={0};
   unsigned int port=1234;


   strncpy(hostname, (const char*)new_con_window.Hostname.mb_str(wxConvUTF8),128);
   port = new_con_window.Port;

    while (vsh==0)
    {

     new_con_window.ShowModal();

     OurName=new_con_window.Nickname;


     if ( new_con_window.IsHost )
      {
        // Host setup
        vsh = Start_VariableSharing("GAMESHARE",hostname,port,"password");
        Add_VariableToSharingList(vsh,"HOST_MOVE",7,&OurMove,sizeof(OurMove));
        Add_VariableToSharingList(vsh,"CLIENT_MOVE",7,&OpponentMove,sizeof(OpponentMove));

      } else
      {
        // Client setup
        vsh = ConnectToRemote_VariableSharing("GAMESHARE",hostname,port,"password");
        Add_VariableToSharingList(vsh,"HOST_MOVE",7,&OpponentMove,sizeof(OpponentMove));
        Add_VariableToSharingList(vsh,"CLIENT_MOVE",7,&OurMove,sizeof(OurMove));
      }


     if ( vsh == 0 )
      {
        wxMessageBox(wxT("Could not connect to peer"),wxT("Please try again"));
      }
    }

}

GamesTesterFrame::~GamesTesterFrame()
{
    //(*Destroy(GamesTesterFrame)
    //*)
}

void GamesTesterFrame::OnQuit(wxCommandEvent& event)
{
    Stop_VariableSharing(vsh);
    Close();
}

void GamesTesterFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg = wxbuildinfo(long_f);
    wxMessageBox(msg, _("Welcome to..."));
}

void GamesTesterFrame::OnSendButtonClick(wxCommandEvent& event)
{
   strncpy(OurMessage, (const char*) OurTextCtrl->GetValue().mb_str(wxConvUTF8),128);


   wxString FinalMessage = OurName;
   FinalMessage << wxT( " : ");
   FinalMessage << OurTextCtrl->GetValue();

   ChatTextCtrl->AppendText(FinalMessage);
}

void GamesTesterFrame::OnClockTimerTrigger(wxTimerEvent& event)
{
   if ( strlen(OpponentMessage)!=0 )
    {
       wxString FinalMessage = OpponentName;
       wxString NewOpponentMessage(OpponentMessage, wxConvUTF8);

       FinalMessage << wxT( " : ");
       FinalMessage << NewOpponentMessage;

       ChatTextCtrl->AppendText(FinalMessage);
    }
}

void GamesTesterFrame::Nudge()
{
 // wxSound msg("Sounds\\nudge.wav",false);
 // msg.Play();
  wxPoint start = GetPosition();
  wxPoint move = start; move.x+=5; move.y-=5; SetPosition(move); wxMilliSleep(50);

  move = start; move.x-=5; move.y+=5; SetPosition(move); wxMilliSleep(50);
  move = start; move.x-=5; move.y-=5; SetPosition(move); wxMilliSleep(50);
  move = start; move.x-=5; move.y+=5; SetPosition(move); wxMilliSleep(50);
  move = start; move.x+=5; move.y+=5; SetPosition(move); wxMilliSleep(50);
  SetPosition (start);
  wxMilliSleep(250);
}

void GamesTesterFrame::OnNudgeButtonClick(wxCommandEvent& event)
{
    Nudge();
}


void DrawField(wxPaintDC * dc)
{
  dc->DrawLine(32,32,32,486); // LEFT
  dc->DrawLine(558,32,558,486); // RIGHT

  dc->DrawLine(32,32,558,32); // UP
  dc->DrawLine(32,486,558,486); // DOWN

}



void GamesTesterFrame::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);


  DrawField(&dc);

  dc.DrawBitmap(OpponentMark,32+0*(67+10),385,true);
  dc.DrawBitmap(OurMark,32+1*(67+10),385,true);

}


