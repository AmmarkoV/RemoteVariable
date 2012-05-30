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

#include <stdio.h>

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

volatile unsigned int OurMove=666;
volatile unsigned int OpponentMove=666;

unsigned int OurScore=0;
unsigned int OpponentScore=0;

volatile unsigned int game_state = 0;
unsigned int new_game = 0;
unsigned int turn = 0;

unsigned int end_line_x1=666;
unsigned int end_line_y1=666;
unsigned int end_line_x2=666;
unsigned int end_line_y2=666;

char OurMessage[128]={0};
char OpponentMessage[128]={0};

unsigned int board[9][9]={0};


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
   EVT_TEXT_ENTER(ID_TEXTCTRL1,GamesTesterFrame::OnSendButtonClick)
   EVT_MOTION(GamesTesterFrame::OnMotion)
END_EVENT_TABLE()

GamesTesterFrame::GamesTesterFrame(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(GamesTesterFrame)
    wxMenuItem* MenuItem2;
    wxMenuItem* MenuItem1;
    wxMenu* Menu1;
    wxMenuBar* MenuBar1;
    wxMenu* Menu2;

    Create(parent, id, _("Games Tester App For RemoteVariables"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    SetClientSize(wxSize(870,562));
    OurTextCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxPoint(600,448), wxSize(256,24), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Label"), wxPoint(784,416), wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    SendButton = new wxButton(this, ID_BUTTON1, _("Send"), wxPoint(752,480), wxSize(104,27), 0, wxDefaultValidator, _T("ID_BUTTON1"));
    ChatTextCtrl = new wxRichTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxPoint(600,18), wxSize(256,424), wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    StaticBox1 = new wxStaticBox(this, ID_STATICBOX1, _("Game area"), wxPoint(16,8), wxSize(568,496), 0, _T("ID_STATICBOX1"));
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



    while (vsh==0)
    {

     new_con_window.ShowModal();

     strncpy(hostname, (const char*)new_con_window.Hostname.mb_str(wxConvUTF8),128);
     fprintf(stderr,"Hostname client got was (%s or %s or %s) \n",hostname,new_con_window.Hostname_cstr,(const char*) new_con_window.Hostname.mb_str());
     port = new_con_window.Port;

     if ( new_con_window.ExitActivated )
      {
         Close();
         return ;
      }

     OurName=new_con_window.Nickname;


     if ( new_con_window.IsHost )
      {
        // Host setup
        turn=1;
        vsh = Start_VariableSharing("GAMESHARE",hostname,port,"password");
        Add_VariableToSharingList(vsh,"HOST_MOVE",7,&OurMove,sizeof(OurMove));
        Add_VariableToSharingList(vsh,"CLIENT_MOVE",7,&OpponentMove,sizeof(OpponentMove));
        Add_VariableToSharingList(vsh,"GAME_STATE",7,&game_state,sizeof(game_state));

        fprintf(stderr,"Waiting for client ");
        while (PeersActive_VariableShare(vsh)==0) {  wxSleep(1); fprintf(stderr,"*");  }

        fprintf(stderr,"Waiting for initialization ");
        game_state=1;
        while (game_state!=2) { wxSleep(1); fprintf(stderr,"."); }
        fprintf(stderr,"\nLets go!\n");

      } else
      {
        // Client setup
        turn=2;
        vsh = ConnectToRemote_VariableSharing("GAMESHARE",hostname,port,"password");
        Add_VariableToSharingList(vsh,"HOST_MOVE",7,&OpponentMove,sizeof(OpponentMove));
        Add_VariableToSharingList(vsh,"CLIENT_MOVE",7,&OurMove,sizeof(OurMove));
        Add_VariableToSharingList(vsh,"GAME_STATE",7,&game_state,sizeof(game_state));

        while (game_state!=1) { wxSleep(1); fprintf(stderr,"."); }
        fprintf(stderr,"\nLets go!\n");
        game_state=2;

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


char FileExists(char * filename)
{
 FILE *fp = fopen(filename,"r");
 if( fp ) { /* exists */
            fclose(fp);
            return 1;
          }
          else
          { /* doesnt exist */ }
 return 0;
}


int PlaySound(char * sndname)
{

  char command_s[1024]={0};
  sprintf(command_s,"src/GamesTester/sounds/%s.wav",sndname);
  if (!FileExists(command_s)) { return 1; }


  sprintf(command_s,"aplay src/GamesTester/sounds/%s.wav&",sndname);
  fprintf(stderr," %s \n ",command_s);
  int i=system((const char * ) command_s);
  return i;
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

   ChatTextCtrl->BeginBold();
   ChatTextCtrl->AppendText(FinalMessage);
   ChatTextCtrl->EndBold();

   FinalMessage = OurTextCtrl->GetValue();
   FinalMessage << wxT("\n");

   ChatTextCtrl->AppendText(FinalMessage);

   OurTextCtrl->SetValue(wxT(""));
}



unsigned int BoardXOfPieceXY(unsigned int x, unsigned int y)
{
  return 32+x*64;
}

unsigned int BoardYOfPieceXY(unsigned int x, unsigned int y)
{
  return 32+y*64;
}


inline int XYOverRect(int x , int y , int rectx1,int recty1,int rectx2,int recty2)
{
  if ( (x>=rectx1) && (x<=rectx2) )
    {
      if ( (y>=recty1) && (y<=recty2) )
        {
          return 1;
        }
    }
  return 0;
}


void CheckEndGame()
{
   /*Horizontal checker*/
   for (int y=0; y<8; y++)
    {
     for (int x=0; x<4; x++)
      {
         if ( (board[x][y]!=0)&&
              (board[x+1][y]==board[x][y])&&
              (board[x+2][y]==board[x][y])&&
              (board[x+3][y]==board[x][y]) )
               {
                 end_line_x1=x; end_line_y1=y; end_line_x2=x+3; end_line_y2=y;
                 fprintf(stderr,"Player %u wins , horizontal line from %u,%u to %u,%u\n",board[x][y],end_line_x1,end_line_y1,end_line_x2,end_line_y2);
                 PlaySound("gong");
               }
      }
    }

   /*Vertical checker*/
   for (int y=0; y<4; y++)
    {
     for (int x=0; x<8; x++)
      {
         if ( (board[x][y]!=0)&&
              (board[x][y+1]==board[x][y])&&
              (board[x][y+2]==board[x][y])&&
              (board[x][y+3]==board[x][y]) )
               {
                 end_line_x1=x; end_line_y1=y; end_line_x2=x; end_line_y2=y+3;
                 fprintf(stderr,"Player %u wins , vertical line from %u,%u to %u,%u\n",board[x][y],end_line_x1,end_line_y1,end_line_x2,end_line_y2);
                 PlaySound("gong");
               }
      }
    }

}


int process_move(unsigned int spot_x , unsigned int player)
{
  if (spot_x>7) { } else
  if (board[spot_x][0]!=0) { } else
         {
           unsigned int spot_y=7;
           while (spot_y>0)
            {
              --spot_y;
              if (board[spot_x][spot_y]==0)
               {
                   board[spot_x][spot_y]=player;
                   PlaySound("put");
                   CheckEndGame();
                   return 1;
               }
            }
           board[spot_x][0]=player;
         }

   PlaySound("wrong");
   return 0;
}

void GamesTesterFrame::OnClockTimerTrigger(wxTimerEvent& event)
{
   if ( strlen(OpponentMessage)!=0 )
    {
       ChatTextCtrl->BeginBold();
       wxString FinalMessage = OpponentName;
       FinalMessage << wxT( " : ");
       ChatTextCtrl->AppendText(FinalMessage);
       ChatTextCtrl->EndBold();

       wxString NewOpponentMessage(OpponentMessage, wxConvUTF8);
       FinalMessage = NewOpponentMessage;
       FinalMessage << wxT("\n");

       ChatTextCtrl->AppendText(FinalMessage);
    }


   if (new_game)
    {
      for (int x=0; x<8; x++)
       {
         for (int y=0; y<8; y++)
         {
            board[x][y]=0;
         }
       }
      turn = 1;
      end_line_x1=666;
      end_line_y1=666;
      end_line_x2=666;
      end_line_y2=666;
      Refresh();
    }


    if (turn==2)
    {
        if ( OpponentMove!=666)
         {
               if ( process_move(OpponentMove,2) )
                {
                  OpponentMove=666;
                  turn=1;
                  Refresh();
                }
         }

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

  dc->DrawLine(32,32,BoardXOfPieceXY(8,0),32); // UP
  dc->DrawLine(32,486,BoardXOfPieceXY(8,0),486); // DOWN

  for ( int x=1; x<9; x++ )
    {
         dc->DrawLine(BoardXOfPieceXY(x,0),32,BoardXOfPieceXY(x,0),486); // UP
    }

}



void DrawPieces(wxPaintDC * dc)
{
  for ( int x=0; x<8; x++ )
    {
      for ( int y=0; y<7; y++ )
      {
         if (board[x][y]==0) {   } else
         if (board[x][y]==1) {   dc->DrawBitmap(OpponentMark,BoardXOfPieceXY(x,y),BoardYOfPieceXY(x,y),true); } else
         if (board[x][y]==2) {   dc->DrawBitmap(OurMark     ,BoardXOfPieceXY(x,y),BoardYOfPieceXY(x,y),true);}
      }
    }
   //dc->DrawLine(558,32,558,486); // RIGHT
}



void GamesTesterFrame::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);


  DrawField(&dc);
  DrawPieces(&dc);

  if ( (end_line_x1!=666) && (end_line_y1!=666) && (end_line_x2!=666) && (end_line_y2!=666) )
   {
        wxPen def_marker(wxColour(0,0,0),1,wxSOLID);
        wxPen red_marker(wxColour(255,0,0),9,wxSOLID);
        dc.SetPen(red_marker);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawLine( BoardXOfPieceXY(end_line_x1,end_line_y1)+32,BoardYOfPieceXY(end_line_x1,end_line_y1)+32 ,
                     BoardXOfPieceXY(end_line_x2,end_line_y2)+32,BoardYOfPieceXY(end_line_x2,end_line_y2)+32  );
        dc.SetPen(def_marker);
   }
}



void GamesTesterFrame::OnMotion(wxMouseEvent& event)
{
  if (turn==2) { return; } // Its other players turn

  int x=event.GetX();
  int y=event.GetY();


   if (( event.LeftIsDown()==1 ) && (OurMove==666) )
   {
        unsigned int pos_x= ( event.GetX() - 32 ) / 64 ;
        unsigned int pos_y= ( event.GetY() - 32 ) / 64;

        if (pos_x>8)
          {
              fprintf(stderr,"X out of bounds %u for mouse pos %u,%u\n",pos_x,x,y);
          } else
         {
              if ( process_move(pos_x,1) )
               {
                 OurMove=pos_x;
                 turn=2;
                 Refresh();
               }
         }
   }

   wxSleep(0.11);

}


