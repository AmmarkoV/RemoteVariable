#ifndef CONNECTION_H
#define CONNECTION_H

//(*Headers(Connection)
#include <wx/gauge.h>
#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//*)

class Connection: public wxDialog
{
	public:

		Connection(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~Connection();

		//(*Declarations(Connection)
		wxButton* ConnectButton;
		wxTextCtrl* NicknameTextCtrl;
		wxStaticText* StaticText1;
		wxGauge* ConnectionProgressGauge;
		wxStaticText* StaticText3;
		wxTextCtrl* PortTextCtrl;
		wxTextCtrl* HostnameTextCtrl;
		wxStaticText* StaticText2;
		wxButton* ExitButton;
		wxChoice* ConnectionTypeChoice;
		//*)

		wxString Hostname;
		wxString Nickname;
		long Port;
		long IsHost;


	protected:

		//(*Identifiers(Connection)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL2;
		static const long ID_GAUGE1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL3;
		//*)

	private:

		//(*Handlers(Connection)
		void OnConnectButtonClick(wxCommandEvent& event);
		void OnExitButtonClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
