#ifndef __EDIT_H__
#define __EDIT_H__

#pragma once

#include <wx/stc/stc.h>

enum ELanguage
{
	Lang_HLSL = 0,
	Lang_ASM
};


class CEditCtrl : public wxStyledTextCtrl
{
public:
	CEditCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
			  const wxPoint& pos = wxDefaultPosition,
			  const wxSize& size = wxDefaultSize
			  );

	~CEditCtrl();


	void SetLanguage(ELanguage lang);


	// Event handlers
	void OnSize(wxSizeEvent& evt);
	void OnKeyEventModifySaveState( wxKeyEvent& evt );

	bool IsSaved() const;
	void SetSaved( bool val );	

private:
	// File
	wxString	m_filename;

	bool		m_bSaved = true;

	wxDECLARE_EVENT_TABLE();
};


#endif