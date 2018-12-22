#pragma once

class CGoToLineDialog : public wxDialog
{
public:
	CGoToLineDialog(wxWindow* parent, int currentLine, int totalLines);

	unsigned int GetLine() const;

private:
	unsigned int m_value = 0;
	
	void OnOK( wxCommandEvent& evt );
	void OnCancel( wxCommandEvent& evt );

	DECLARE_EVENT_TABLE()	
};