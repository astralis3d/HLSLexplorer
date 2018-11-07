#pragma once

class CAboutDlg : public wxDialog
{
public:
	CAboutDlg(wxWindow* parent);

private:
	void OnOK(wxCommandEvent& evt);

	wxDECLARE_EVENT_TABLE();
};