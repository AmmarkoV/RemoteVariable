/***************************************************************
 * Name:      GamesTesterApp.cpp
 * Purpose:   Code for Application Class
 * Author:    Ammar Qammaz (ammarkov@gmail.com)
 * Created:   2012-05-28
 * Copyright: Ammar Qammaz (http://ammar.gr)
 * License:
 **************************************************************/

#include "GamesTesterApp.h"

//(*AppHeaders
#include "GamesTesterMain.h"
#include <wx/image.h>
//*)

IMPLEMENT_APP(GamesTesterApp);

bool GamesTesterApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	GamesTesterFrame* Frame = new GamesTesterFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}
