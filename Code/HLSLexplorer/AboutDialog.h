#pragma once

class CAboutDialog : public wxDialog
{
public:
	CAboutDialog(wxWindow* parent);

private:
	void OnOK(wxCommandEvent& evt);

	wxDECLARE_EVENT_TABLE();
};