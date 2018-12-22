#include "PCH.h"
#include "GoToLineDialog.h"
#include <wx/xrc/xmlres.h>
#include <wx/valnum.h>


BEGIN_EVENT_TABLE(CGoToLineDialog, wxDialog)

EVT_BUTTON( XRCID("OK"), CGoToLineDialog::OnOK )
EVT_BUTTON( XRCID("Cancel"), CGoToLineDialog::OnCancel)

END_EVENT_TABLE()

//-----------------------------------------------------------------------------
CGoToLineDialog::CGoToLineDialog( wxWindow* parent, int currentLine, int totalLines )
	: m_value(0)
{
	wxXmlResource::Get()->LoadDialog( this, parent, wxT("GoToLineDialog") );

	
	// Prepare validator for text control
	wxIntegerValidator<unsigned int> val(&m_value);
	val.SetMin(1);
	val.SetMax(totalLines);

	// don't want to beep if invalid key is pressed
	wxValidator::SuppressBellOnError(true);
		
	// Assign validator to text control
	wxTextCtrl* pEditLineNumber = XRCCTRL(*this, "m_editLineNumber", wxTextCtrl);
	pEditLineNumber->SetValidator(val);

	m_value = currentLine;

	wxStaticText* hintText = XRCCTRL(*this, "m_staticLineNumber", wxStaticText);
	hintText->SetLabel( wxString::Format("Line number (1 - %d):", totalLines) );
	
	// What happens when user presses Esc
	SetEscapeId(XRCID("Cancel"));

	SetSize(300, -1);
}

//------------------------------------------------------------------------
unsigned int CGoToLineDialog::GetLine() const
{
	wxTextCtrl* pEditLineNumber = XRCCTRL(*this, "m_editLineNumber", wxTextCtrl);
	pEditLineNumber->GetValidator()->TransferFromWindow();

	return m_value;
}

//------------------------------------------------------------------------
void CGoToLineDialog::OnOK( wxCommandEvent& WXUNUSED( evt ) )
{
	if (IsModal())
	{
		EndModal( wxID_OK );
	}
	else
	{
		Show( false );
	}
}

//------------------------------------------------------------------------
void CGoToLineDialog::OnCancel( wxCommandEvent& WXUNUSED( evt ) )
{
	if (IsModal())
	{
		EndModal( wxID_CANCEL );
	}
	else
	{
		Show( false );
	}
}