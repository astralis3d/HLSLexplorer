#pragma once


class CNewPresetDialog : public wxDialog
{
public:
	CNewPresetDialog(wxWindow* parent);

	wxString GetPresetName() const;

	void OnOK(wxCommandEvent& evt);
	void OnCancel(wxCommandEvent& evt);

private:
	wxString m_presetName = wxEmptyString;

	wxDECLARE_EVENT_TABLE();
};