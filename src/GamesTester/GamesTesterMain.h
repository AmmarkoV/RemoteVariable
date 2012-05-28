/***************************************************************
 * Name:      GamesTesterMain.h
 * Purpose:   Defines Application Frame
 * Author:    Ammar Qammaz (ammarkov@gmail.com)
 * Created:   2012-05-28
 * Copyright: Ammar Qammaz (http://ammar.gr)
 * License:
 **************************************************************/

#ifndef GAMESTESTERMAIN_H
#define GAMESTESTERMAIN_H

//(*Headers(GamesTesterFrame)
#include <wx/button.h>
#include <wx/menu.h>
#include <wx/statusbr.h>
#include <wx/statbox.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class GamesTesterFrame: public wxFrame
{
    public:

        GamesTesterFrame(wxWindow* parent,wxWindowID id = -1);
        virtual ~GamesTesterFrame();

    private:

        //(*Handlers(GamesTesterFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        //*)

        //(*Identifiers(GamesTesterFrame)
        static const long ID_TEXTCTRL1;
        static const long ID_STATICTEXT1;
        static const long ID_BUTTON1;
        static const long ID_TEXTCTRL2;
        static const long ID_STATICBOX1;
        static const long idMenuQuit;
        static const long idMenuAbout;
        static const long ID_STATUSBAR1;
        //*)

        //(*Declarations(GamesTesterFrame)
        wxStatusBar* StatusBar1;
        wxStaticText* StaticText1;
        wxTextCtrl* OurTextCtrl;
        wxStaticBox* StaticBox1;
        wxTextCtrl* TextCtrl1;
        wxButton* SendButton;
        //*)

        DECLARE_EVENT_TABLE()
};

#endif // GAMESTESTERMAIN_H
