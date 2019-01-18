#include "PCH.h"
#include "Edit.h"
#include "Prefs.h"

BEGIN_EVENT_TABLE(CEditCtrl, wxStyledTextCtrl)
	EVT_SIZE(CEditCtrl::OnSize)
	EVT_KEY_DOWN(CEditCtrl::OnKeyEventModifySaveState )

	EVT_STC_CHARADDED(wxID_ANY, CEditCtrl::OnCharAdded)
END_EVENT_TABLE()

CEditCtrl::CEditCtrl(wxWindow* parent, wxWindowID id /*= wxID_ANY*/,
					 const wxPoint& pos /*= wxDefaultPosition*/,
					 const wxSize& size /*= wxDefaultSize*/)
	: wxStyledTextCtrl(parent, id, pos, size)
{
	m_filename = wxEmptyString;

	StyleClearAll();

	// Use all the bits in the style byte as styles, not indicators.
	//SetStyleBits(8);


	wxFont font( wxFontInfo(10).FaceName("Consolas").Family(wxFONTFAMILY_MODERN) );
	StyleSetFont(wxSTC_STYLE_DEFAULT, font);

	StyleSetForeground(wxSTC_STYLE_DEFAULT, *wxBLACK);
	StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxWHITE);
	StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(wxT("DARK GREY")));
	StyleSetBackground(wxSTC_STYLE_LINENUMBER, *wxWHITE);
	StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(wxT("DARK GREY")));

	SetMarginWidth(0, 25);
	StyleSetForeground( wxSTC_STYLE_LINENUMBER, wxColour( 75, 75, 75 ) );
	StyleSetBackground( wxSTC_STYLE_LINENUMBER, wxColour( 220, 220, 220 ) );
	SetMarginType(0, wxSTC_MARGIN_NUMBER);

}

CEditCtrl::~CEditCtrl()
{

}

//------------------------------------------------------------------------
void CEditCtrl::SetLanguage(ELanguage lang)
{
	// Init style
	LanguageInfo const* curInfo = NULL;
	curInfo = &g_LanguagePrefs[lang];

	SetLexer(curInfo->lexer);


	int keywordNr = 0;
	for (int i = 0; i < 32; ++i)
	{
		if (curInfo->styles[i].type == (-1))
			continue;

		const StyleInfo& curType = g_StylePrefs[curInfo->styles[i].type];

		wxFont font(wxFontInfo(10).FaceName("Consolas").Family(wxFONTFAMILY_MODERN));
		StyleSetFont(i, font);

		if (curType.background)
			StyleSetBackground(i, wxColour(curType.background));
		if (curType.foreground)
			StyleSetForeground(i, wxColour(curType.foreground));

		StyleSetBold(i, (curType.fontstyle & mySTC_STYLE_BOLD) > 0);
		StyleSetItalic(i, (curType.fontstyle & mySTC_STYLE_ITALIC) > 0);
		StyleSetUnderline(i, (curType.fontstyle & mySTC_STYLE_UNDERL) > 0);
		StyleSetVisible(i, (curType.fontstyle & mySTC_STYLE_HIDDEN) == 0);
		StyleSetCase(i, curType.lettercase);

		const char* pwords = curInfo->styles[i].words;
		if (pwords)
		{
			SetKeyWords(keywordNr++, pwords);
		}

	}

	SetTabWidth(4);
	SetUseTabs(false);
}

//-----------------------------------------------------------------------------
void CEditCtrl::OnCharAdded( wxStyledTextEvent & evt )
{
	const char chr = (char) evt.GetKey();
	const int currLine = GetCurrentLine();

	if (chr == '\n')
	{
		int lineInd = 0;
		if (currLine > 0)
		{
			// Get indentation of previous line
			lineInd = GetLineIndentation( currLine - 1 );
		}
		
		if (lineInd == 0)
			return;

		// Set indentation from previous line for new one
		SetLineIndentation( currLine, lineInd );
		GotoPos( PositionFromLine(currLine) + lineInd );
	}
}

//------------------------------------------------------------------------
void CEditCtrl::OnSize(wxSizeEvent& evt)
{
	evt.Skip();
}

//-----------------------------------------------------------------------------
void CEditCtrl::OnKeyEventModifySaveState( wxKeyEvent& evt )
{
	m_bSaved = false;

	evt.Skip();
}

bool CEditCtrl::IsSaved() const
{
	return m_bSaved;
}

void CEditCtrl::SetSaved( bool val )
{
	m_bSaved = val;
}
