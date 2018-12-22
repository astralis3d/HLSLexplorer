#include "PCH.h"
#include "MainFrame.h"
#include "Edit.h"
#include "ControlsPanel.h"
#include "AboutDialog.h"
#include "GoToLineDialog.h"
#include "RealtimePSPreviewFrame.h"
#include "CompilationDX.h"

#include "DummyShaders.h"

#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/xrc/xmlres.h>

wxBEGIN_EVENT_TABLE( CMyFrame, wxFrame )

	EVT_CLOSE( CMyFrame::OnCloseEvent )

	EVT_MENU( ID_TOGGLEPANELVISIBILITY, CMyFrame::OnMenuFileTogglePanelVisiblity )
	EVT_MENU( ID_COMPILE, CMyFrame::OnMenuFileCompile )
	EVT_MENU( ID_LOAD_D3DCOMPILER_DLL, CMyFrame::OnMenuFileLoadD3DCompilerDLL)
	EVT_MENU( ID_OPEN_HLSL, CMyFrame::OnMenuFileOpenHLSLShader )
	EVT_MENU( ID_SAVE_AUTO, CMyFrame::OnMenuFileSaveAuto )
	EVT_MENU( ID_SAVE_HLSL_SHADER, CMyFrame::OnMenuFileSaveHLSLShader )
	EVT_MENU( ID_SAVE_DISASSEMBLED_SHADER, CMyFrame::OnMenuFileSaveDisassembledShader )
	EVT_MENU( ID_REALTIME_PIXEL_SHADER_PREVIEW, CMyFrame::OnMenuFileShowPSPreview )
	EVT_MENU( ID_FILE_EXIT, CMyFrame::OnMenuFileExit )

	EVT_MENU( ID_EDIT_GOTO_LINE, CMyFrame::OnMenuEditGoToLine )

	EVT_MENU( ID_INSERT_DUMMY_VERTEX_SHADER, CMyFrame::OnMenuInsertDummyVS )
	EVT_MENU( ID_INSERT_DUMMY_PIXEL_SHADER, CMyFrame::OnMenuInsertDummyPS )
	EVT_MENU( ID_INSERT_DUMMY_COMPUTE_SHADER, CMyFrame::OnMenuInsertDummyCS )

	EVT_MENU( ID_SETTINGS_AUTO_WINDOW_SPLIT, CMyFrame::OnMenuSettingsAutoWindowSplit )

	EVT_MENU( ID_ABOUT, CMyFrame::OnAbout )

	EVT_SIZE( CMyFrame::OnSize )
wxEND_EVENT_TABLE()


CMyFrame::CMyFrame( const wxString& str )
	: wxFrame( nullptr, wxID_ANY, str, wxDefaultPosition, wxSize(1280, 800) )
{
	InitializeMenu();
	InitializeUI();
}

//------------------------------------------------------------------------
void CMyFrame::InitializeUI()
{
	// Main panel
	m_mainPanel = new wxPanel( this, wxID_ANY );

	//! Initialize main splitter
	m_pSplitter = new wxSplitterWindow( m_mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE );
	
	//! Left window. Also bind enter/leave events
	m_pEditHLSL = new CEditCtrl( m_pSplitter, wxID_ANY, wxDefaultPosition, wxSize( -1, 500 ) );

	// left window is active when cursor enters it, or is left-clicked.
	m_pEditHLSL->Bind( wxEVT_LEFT_DOWN, &CMyFrame::OnMouseLeftWindowActive, this );
//	m_pEditHLSL->Bind( wxEVT_ENTER_WINDOW, &CMyFrame::OnMouseLeftWindowActive, this );
	m_pEditHLSL->SetLanguage( Lang_HLSL );
	m_pEditSelected = m_pEditHLSL;


	//! Right window
	m_pRightWindow = CreateDisassemblerOutputNotebook( m_pSplitter );

	// right window is active (in this case Ctrl+S saves disassembled shader):
	m_pRightWindow->Bind( wxEVT_ENTER_WINDOW, &CMyFrame::OnMouseRightWindowDXBCActive, this );

	m_pEditASM_DXBC->Bind( wxEVT_LEFT_DOWN, &CMyFrame::OnMouseRightWindowDXBCActive, this );
	m_pEditASM_DXIL->Bind( wxEVT_LEFT_DOWN, &CMyFrame::OnMouseRightWindowDXILActive, this );
	m_pEditASM_GCNISA->Bind( wxEVT_LEFT_DOWN, &CMyFrame::OnMouseRightWindowGCNISAActive, this );

	//m_pEditASM_DXBC->Bind( wxEVT_ENTER_WINDOW, &CMyFrame::OnMouseRightWindowActive, this );
	//m_pEditASM_DXIL->Bind( wxEVT_ENTER_WINDOW, &CMyFrame::OnMouseRightWindowActive, this );
	//m_pEditASM_GCNISA->Bind( wxEVT_ENTER_WINDOW, &CMyFrame::OnMouseRightWindowActive, this );

	// !Setup main wxSplitterWindow
	m_pSplitter->SetMinimumPaneSize( 100 );
	m_pSplitter->SetSashPosition( 500 );
	m_pSplitter->SplitVertically( m_pEditHLSL, m_pRightWindow, 0 );
	m_pSplitter->SetSize( GetClientSize() );


	// Code windows sizer
	wxBoxSizer* pSizerEditorsParent = new wxBoxSizer( wxVERTICAL );
	pSizerEditorsParent->Add( m_pSplitter, 1, wxEXPAND | wxALL, 0 );


	// Controls panel sizer.
	m_pControlsPanel = new CControlsPanel( m_mainPanel, &m_D3DOptions );
	wxBoxSizer* pSizerControls = new wxBoxSizer( wxVERTICAL );
	pSizerControls->Add( m_pControlsPanel, 0, wxEXPAND | wxALL );

	// Top-level sizer
	wxBoxSizer* pTopSizer = new wxBoxSizer( wxVERTICAL );
	pTopSizer->Add( pSizerEditorsParent, 1, wxEXPAND | wxALL, 0 );
	pTopSizer->Add( pSizerControls, 0, wxALL, 2 );

	m_mainPanel->SetSizer( pTopSizer );
}

//------------------------------------------------------------------------
void CMyFrame::InitializeMenu()
{
	// file
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append( ID_LOAD_D3DCOMPILER_DLL, "Load D3DCompiler DLL..." );
	fileMenu->AppendSeparator();
	fileMenu->Append( ID_OPEN_HLSL, "Open...\tCtrl+O" );
	fileMenu->Append( ID_SAVE_AUTO, "Save...\tCtrl+S" );
	fileMenu->AppendSeparator();
	fileMenu->Append( ID_COMPILE, "Compile\tF5" );
	fileMenu->AppendCheckItem( ID_TOGGLEPANELVISIBILITY, "Show panel\tF6" );
	fileMenu->Append( ID_REALTIME_PIXEL_SHADER_PREVIEW, "Real-time PS preview...\tF7" );
	fileMenu->Check( ID_TOGGLEPANELVISIBILITY, true );
	fileMenu->AppendSeparator();
	fileMenu->Append( ID_FILE_EXIT, "E&xit\tAlt+F4", "Quit this program" );

	// edit
	wxMenu* editMenu = new wxMenu;
	editMenu->Append( ID_EDIT_GOTO_LINE, "Go To Line...\tCtrl+G" );

	// insert
	wxMenu* insertMenu = new wxMenu;
	insertMenu->Append( ID_INSERT_DUMMY_VERTEX_SHADER, "Insert simple VS" );
	insertMenu->Append( ID_INSERT_DUMMY_PIXEL_SHADER, "Insert simple PS" );
	insertMenu->Append( ID_INSERT_DUMMY_COMPUTE_SHADER, "Insert simple CS" );

	// settings
	wxMenu* settingsMenu = new wxMenu;
	settingsMenu->AppendCheckItem( ID_SETTINGS_AUTO_WINDOW_SPLIT, wxT("Split windows automatically\tF10") );
	settingsMenu->Check( ID_SETTINGS_AUTO_WINDOW_SPLIT, true );

	// help
	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append( ID_ABOUT, "&About...\tF1", "Show about dialog" );



	// now append the freshly created menu to the menu bar...
	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append( fileMenu, "&File" );
	menuBar->Append( editMenu, "&Edit" );
	menuBar->Append( insertMenu, "&Insert" );
	menuBar->Append( settingsMenu, "&Settings" );
	menuBar->Append( helpMenu, "&Help" );

	// ... and attach this menu bar to the frame
	SetMenuBar( menuBar );
}

//------------------------------------------------------------------------
wxNotebook* CMyFrame::CreateDisassemblerOutputNotebook(wxWindow* parent)
{
	wxNotebook* pNotebook = new wxNotebook( parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 500) );

	// DXBC
	m_pEditASM_DXBC = new CEditCtrl( pNotebook, wxID_ANY );
	m_pEditASM_DXBC->SetLanguage( Lang_ASM );

	// DXIL - Shader Model 6.0+
	m_pEditASM_DXIL = new CEditCtrl( pNotebook, wxID_ANY );
	m_pEditASM_DXIL->SetLanguage( Lang_ASM );

	// AMD GCN ISA
	m_pEditASM_GCNISA = new CEditCtrl( pNotebook, wxID_ANY );
	m_pEditASM_GCNISA->SetLanguage( Lang_ASM );

	// add pages
	pNotebook->AddPage(m_pEditASM_DXBC, wxT("DXBC"), true );
	pNotebook->AddPage(m_pEditASM_DXIL, wxT("DXIL"), false );
	pNotebook->AddPage(m_pEditASM_GCNISA, wxT("AMD GCN ISA"), false );

	return pNotebook;
}

//------------------------------------------------------------------------
void CMyFrame::OnAbout( wxCommandEvent& WXUNUSED( event ) )
{
	CAboutDlg aboutDlg(this);
	aboutDlg.ShowModal();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuFileExit( wxCommandEvent& WXUNUSED( event ) )
{
	// pump up close event with option to cancel close request.
	Close(false);
}

//------------------------------------------------------------------------
void CMyFrame::OnSize( wxSizeEvent& event )
{
	if (m_pSplitter)
	{
		if (m_bAutoWindowSplit)
		{
			const int mediumX = GetClientSize().x / 2;
			m_pSplitter->SetSashPosition( mediumX );
		}	
	}

	event.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuFileTogglePanelVisiblity( wxCommandEvent& WXUNUSED(evt) )
{
	wxSizer* const pSizer = m_mainPanel->GetSizer();

	pSizer->Show( 1, !m_pControlsPanel->IsShown() );
	pSizer->Layout();
}
//------------------------------------------------------------------------
void CMyFrame::OnMenuFileLoadD3DCompilerDLL( wxCommandEvent& WXUNUSED(evt) )
{
	wxString path = wxFileSelector("Choose a D3DCompiler dll", wxEmptyString, wxEmptyString, "dll", wxT("DLL files|*.dll"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if ( !path.empty() )
	{
		if ( !m_compilerLoader.LoadD3DCompilerDLL( path.c_str().AsChar() ) )
		{
			wxMessageBox( wxT("Failed to load d3dcompiler."), wxT("Warning"), wxOK | wxCENTRE | wxICON_WARNING );
		}
	}
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuFileCompile( wxCommandEvent& evt )
{
	// Entrypoint from textctrl
	wxTextCtrl* pTextctrlEntrypoint = XRCCTRL( *this, "textEntrypoint", wxTextCtrl );
	const wxString strEntrypoint = pTextctrlEntrypoint->GetValue();

	// Text from editor
	wxString sourceHLSL = m_pEditHLSL->GetText();
	const char* pszSourceHLSL = sourceHLSL.c_str();

	if ( m_compilerLoader.IsValid() )
	{
		m_pEditASM_DXBC->SetText(wxString("Please wait..."));

		// This is out DXBC data
		std::vector<unsigned char> compiledDXBC;
			
		const std::string strCompiledASM = nmCompile::Compile( m_D3DOptions, pszSourceHLSL, strEntrypoint.c_str(), m_strHLSLPath.c_str(), &m_compilerLoader, compiledDXBC );
		const wxString wstrCompiledASM = wxString( strCompiledASM );

		m_pEditASM_DXBC->SetText( wstrCompiledASM );

		std::string strCompiledGCNISA;	
		if (compiledDXBC.size() > 0)
		{
			// Send to AMD GCN ISA to get assembly
			const E_ASIC_TYPE asicType = m_pControlsPanel->GetSelectedAsicType();
			strCompiledGCNISA = m_gcnisa.Compile( compiledDXBC.data(), compiledDXBC.size(), asicType );

			// If real-time Pixel Shader preview is visible, update it
			if ( m_bPSPreviewVisible && m_D3DOptions.shaderType == EShaderType::ShaderType_PS )
			{
				m_pPSPreviewFrame->GetD3D11Driver()->CreatePixelShader( (const void*) compiledDXBC.data(), compiledDXBC.size() );
			}
		}

		m_pEditASM_GCNISA->SetText( strCompiledGCNISA );
	}	
	else
	{	
		wxMessageBox( "D3Dcompiler dll is not loaded. Please locate valid dll manually.", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_WARNING, 0 );
	}

	// For modern compiler SM6.0+
	if ( m_modernCompierLoader.IsValid() )
	{
		// Compiled ASM for Modern DXBC
		const std::string strCompiledASM = nmCompile::CompileModern( m_D3DOptions, pszSourceHLSL, strEntrypoint.wc_str(), m_strHLSLPath.c_str());
		m_pEditASM_DXIL->SetText( strCompiledASM );

		// If user selected shader model 6.0+, go to this tab immediately.
		if ( m_D3DOptions.shaderProfile == EShaderProfile::ShaderProfile_6_0 ||
			 m_D3DOptions.shaderProfile == EShaderProfile::ShaderProfile_6_1 ||
			 m_D3DOptions.shaderProfile == EShaderProfile::ShaderProfile_6_2 ||
			 m_D3DOptions.shaderProfile == EShaderProfile::ShaderProfile_6_3 )
		{
			m_pRightWindow->SetSelection( 1 );
		}
	}

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuFileShowPSPreview( wxCommandEvent& evt )
{
	// Do nothing if PS preview is already active
	if (m_bPSPreviewVisible)
	{
		//OutputDebugStringA("No2 previews\n");

		m_pPSPreviewFrame->SetFocus();

		return;
	}

	m_pPSPreviewFrame = new CRealtimePSPreviewFrame( this );
	m_pPSPreviewFrame->SetVisibilityPtr( &m_bPSPreviewVisible );
	m_pPSPreviewFrame->InitD3D11();
	m_pPSPreviewFrame->Show( true );
	m_pPSPreviewFrame->SetFocus();
	m_bPSPreviewVisible = true;

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuFileOpenHLSLShader( wxCommandEvent& evt )
{
	wxFileDialog fileDlg( this, wxT( "Select HLSL file to open..." ), wxEmptyString, wxEmptyString, "HLSL files(*.hlsl)|*.hlsl", wxFD_OPEN | wxFD_FILE_MUST_EXIST );

	if (fileDlg.ShowModal() == wxID_CANCEL)
	{
		return;
	}

	// Get path to opened HLSL shader
	wxString fileFullPath =  fileDlg.GetPath();
	const size_t x = fileFullPath.find_last_of("\\/");
	m_strHLSLPath = fileFullPath.substr(0, x);
	

	wxFileInputStream inStream( fileDlg.GetPath() );
	if (!inStream.IsOk())
	{
		// todo: log error
		return;
	}

	wxStringOutputStream str;
	inStream.Read(str);
	
	m_pEditHLSL->SetText( str.GetString() );

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuFileSaveAuto( wxCommandEvent& evt )
{
	CEditCtrl* pCodeWindow = nullptr;

	// Save shader to file
	if (m_bHLSLWindowActive)
	{
		pCodeWindow = m_pEditHLSL;

		wxFileDialog fileDlg(this, wxT("Save HLSL source..."), wxEmptyString, wxEmptyString, "HLSL files(*.hlsl)|*.hlsl", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (fileDlg.ShowModal() == wxID_CANCEL)
		{
			return;
		}
		else
		{
			wxFileOutputStream outStream( fileDlg.GetPath() );
			if (!outStream.IsOk())
			{
				// Error here
				return;
			}

			// save bytes
			outStream.Write( (const void*)pCodeWindow->GetText().c_str(), pCodeWindow->GetText().size() );
			
			// set hlsl state to saved
			pCodeWindow->SetSaved( true );
		}
	}
	else if (m_bASMWindowActive)		// // Save disassembled output to file
	{
		// get wxNotebook
		wxNotebook* pNotebookASM = (wxNotebook*) m_pSplitter->GetWindow2();
		wxString asmTypeString = wxEmptyString;

		// assign proper window
		const int currSelection = pNotebookASM->GetSelection();
		switch (currSelection)
		{
			// DXBC
			case 0:
			{
				pCodeWindow = m_pEditASM_DXBC;
				asmTypeString = wxT("DXBC");			
			}
			break;

			// DXIL
			case 1:
			{
				pCodeWindow = m_pEditASM_DXIL;
				asmTypeString = wxT("DXIL");
			}
			break;
				
			// AMD GCN ISA
			case 2:	
			{
				pCodeWindow = m_pEditASM_GCNISA;
				asmTypeString = wxT("AMD GCN ISA");
			
			}	break;


			default:
				break;	// uh-oh, shouldn't happen
		}

		if (!pCodeWindow)
			return;

		wxFileDialog fileDlg( this,
							  wxString::Format("Save disassembled %s source...", asmTypeString),
							  wxEmptyString, 
							  wxEmptyString,
							  "TXT files(*.txt)|*.txt", 
							  wxFD_SAVE | wxFD_OVERWRITE_PROMPT );

		if (fileDlg.ShowModal() == wxID_CANCEL)
		{
			return;
		}
		else
		{
			wxFileOutputStream outStream( fileDlg.GetPath() );
			if (!outStream.IsOk())
			{
				// Error here
				return;
			}

			// Save bytes
			outStream.Write( (const void*) pCodeWindow->GetText().c_str(), pCodeWindow->GetText().size() );
		}
	}

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuFileSaveHLSLShader( wxCommandEvent& evt )
{
	// not implemented yet...

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuFileSaveDisassembledShader( wxCommandEvent& evt )
{
	// not implemented yet...

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuEditGoToLine( wxCommandEvent& evt )
{
	if (!m_pEditSelected)
		return;

	// Scintilla starts numbering lines from 0.
	const int currentLine = m_pEditSelected->GetCurrentLine() + 1;
	const int totalLines = m_pEditSelected->GetLineCount();

	CGoToLineDialog dialog(this, currentLine, totalLines);
	if (dialog.ShowModal() == wxID_OK)
	{
		const int lineGoTo = dialog.GetLine() - 1;

		if (lineGoTo >= 0)
		{
			m_pEditSelected->GotoLine(lineGoTo);
		}		
	}

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuInsertDummyVS( wxCommandEvent& WXUNUSED( evt ) )
{
	wxTextCtrl* pEntrypoint = XRCCTRL(*this, "textEntrypoint", wxTextCtrl);
	if (pEntrypoint)
	{
		pEntrypoint->SetValue("DummyVS");
	}

	wxRadioBox* pShaderTypeRadiobox = XRCCTRL(*this, "radioboxShaderType", wxRadioBox);
	if (pShaderTypeRadiobox)
	{
		pShaderTypeRadiobox->SetSelection(0);
		m_D3DOptions.shaderType = ShaderType_VS;
	}

	m_pEditHLSL->SetText(szDummyVS);
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuInsertDummyPS( wxCommandEvent& WXUNUSED( evt ) )
{
	wxTextCtrl* pEntrypoint = XRCCTRL( *this, "textEntrypoint", wxTextCtrl );
	if (pEntrypoint)
	{
		pEntrypoint->SetValue( "DummyPS" );
	}


	wxRadioBox* pShaderTypeRadiobox = XRCCTRL( *this, "radioboxShaderType", wxRadioBox );
	if (pShaderTypeRadiobox)
	{
		pShaderTypeRadiobox->SetSelection( 2 );
		m_D3DOptions.shaderType = ShaderType_PS;
	}
	
	m_pEditHLSL->SetText( szDummyPS );
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuInsertDummyCS( wxCommandEvent& WXUNUSED(evt) )
{
	wxTextCtrl* pEntrypoint = XRCCTRL( *this, "textEntrypoint", wxTextCtrl );
	if (pEntrypoint)
	{
		pEntrypoint->SetValue( "DummyCS" );
	}

	wxRadioBox* pShaderTypeRadiobox = XRCCTRL( *this, "radioboxShaderType", wxRadioBox );
	if (pShaderTypeRadiobox)
	{
		pShaderTypeRadiobox->SetSelection( 5 );
		m_D3DOptions.shaderType = ShaderType_CS;
	}

	m_pEditHLSL->SetText( szDummyCS );
}

//------------------------------------------------------------------------
void CMyFrame::OnCloseEvent( wxCloseEvent& evt )
{
	if ( evt.CanVeto() )
	{
		if ( !m_pEditHLSL->IsSaved() )
		{
			wxMessageDialog dlg( this,
								 "Do you want to save your HLSL before exit?",
								 "Question",
								 wxCENTER | wxYES_NO | wxYES_DEFAULT | wxCANCEL | wxICON_QUESTION );

			const int answer = dlg.ShowModal();
			switch (answer)
			{
				case wxID_CANCEL:
				{
					// Back to program
					evt.Veto();
					return;
				}
				break;

				case wxID_NO:
				{
					// Exit
					Destroy();
				}
				break;

				case wxID_YES:
				{
					// Save shit (if user wants to) and exit
					wxFileDialog fileDlg( this, wxT( "Save HLSL source..." ), wxEmptyString, wxEmptyString, "HLSL files(*.hlsl)|*.hlsl", wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
					if (fileDlg.ShowModal() == wxID_CANCEL)
					{
						evt.Veto();
						return;
					}
					else
					{
						wxFileOutputStream outStream( fileDlg.GetPath() );
						if (!outStream.IsOk())
						{
							// Error here
							return;
						}

						// save bytes
						outStream.Write( (const void*)m_pEditHLSL->GetText().c_str(), m_pEditHLSL->GetText().size() );

						Destroy();
					}
				}
				break;
			}
		}  // if ( !m_pEditHLSL->IsSaved() )
		else
		{
			Destroy();
		}
	}
	else
	{
		Destroy();
	}
}

//------------------------------------------------------------------------
void CMyFrame::OnMenuSettingsAutoWindowSplit( wxCommandEvent& evt )
{
	m_bAutoWindowSplit = evt.IsChecked();

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMouseLeftWindowActive( wxMouseEvent& evt )
{
	m_bHLSLWindowActive = true;
	m_bASMWindowActive = false;

	m_pEditSelected = m_pEditHLSL;

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMouseRightWindowDXBCActive( wxMouseEvent& evt )
{
	m_bHLSLWindowActive = false;
	m_bASMWindowActive = true;

	m_pEditSelected = m_pEditASM_DXBC;

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMouseRightWindowDXILActive( wxMouseEvent& evt )
{
	m_bHLSLWindowActive = false;
	m_bASMWindowActive = true;

	m_pEditSelected = m_pEditASM_DXIL;

	evt.Skip();
}

//------------------------------------------------------------------------
void CMyFrame::OnMouseRightWindowGCNISAActive( wxMouseEvent& evt )
{
	m_bHLSLWindowActive = false;
	m_bASMWindowActive = true;

	m_pEditSelected = m_pEditASM_GCNISA;

	evt.Skip();
}
