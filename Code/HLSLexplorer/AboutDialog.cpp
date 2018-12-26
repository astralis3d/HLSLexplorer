#include "PCH.h"
#include "AboutDialog.h"
#include <wx/xrc/xmlres.h>

BEGIN_EVENT_TABLE(CAboutDialog, wxDialog )
	EVT_BUTTON( XRCID( "btnOK" ), CAboutDialog::OnOK )
END_EVENT_TABLE()

CAboutDialog::CAboutDialog( wxWindow* parent )
{
	wxXmlResource::Get()->LoadDialog( this, parent, wxT( "AboutDlg" ) );
	
	// assign other stuff
	wxStaticText* pText = XRCCTRL(*this, "m_staticVersion", wxStaticText);
	pText->SetLabel( wxT("Version: 1.0") );

	// license & legal stuff
	{
		wxTextCtrl* pEdit = XRCCTRL( *this, "m_staticLicense", wxTextCtrl );

		const wxString info = wxString::Format(
			"This software uses third party libraries:\n"
			"- d3dcompiler (c) Microsoft Corporation\n"
			"- Radeon DirectX 11 Driver (c) Advanced Micro Devices, Inc.\n"
			"- elf32.h library (c) John D. Polstra\n"
			" -elf_common.h (c) John D. Polstra, David E. O'Brien, Dell EMC\n"
			"- %s (c) wxWidgets dev team\n",
			wxVERSION_STRING );

		pEdit->Clear();
		pEdit->WriteText( info );
	}

	// focus to OK button
	{
		wxButton* pOKBtn = XRCCTRL(*this, "btnOK", wxButton);
		pOKBtn->SetFocus();
		pOKBtn->SetDefault();
	}

	// Exit with escape
	SetEscapeId(XRCID("btnOK"));
}

void CAboutDialog::OnOK( wxCommandEvent& evt )
{
	if ( IsModal() )
	{
		EndModal( wxID_OK );
	}
	else
	{
		Show(false);
	}

	evt.Skip();
}