#include "PCH.h"
#include "NewPresetDialog.h"

#include <wx/xrc/xmlres.h>

BEGIN_EVENT_TABLE(CNewPresetDialog, wxDialog)

EVT_BUTTON( XRCID("btnOK"), CNewPresetDialog::OnOK )
EVT_BUTTON( XRCID("btnCancel"), CNewPresetDialog::OnCancel )

END_EVENT_TABLE()

CNewPresetDialog::CNewPresetDialog( wxWindow* parent )
{
	wxXmlResource::Get()->LoadDialog(this, parent, wxT("nameDialog") );

	SetEscapeId(XRCID("btnCancel"));
}

//------------------------------------------------------------------------
wxString CNewPresetDialog::GetPresetName() const
{
	return m_presetName;
}

//------------------------------------------------------------------------
void CNewPresetDialog::OnOK( wxCommandEvent& WXUNUSED(evt) )
{
	wxTextCtrl* pTextCtrl = XRCCTRL(*this, "editPresetName", wxTextCtrl);
	if (!pTextCtrl)
		return;

	m_presetName = pTextCtrl->GetValue();
	
	if (!m_presetName.empty())
	{
		EndModal( wxID_OK );
	}
	else
	{
		EndModal( wxID_CANCEL );
	}
}

//------------------------------------------------------------------------
void CNewPresetDialog::OnCancel( wxCommandEvent& WXUNUSED(evt) )
{
	EndModal(wxID_CANCEL);
}