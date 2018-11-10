#include "PCH.h"
#include "RealtimePSPreviewFrame.h"

#include <wx/splitter.h>
#include <wx/filepicker.h>
#include <wx/xrc/xmlres.h>

BEGIN_EVENT_TABLE( CRealtimePSPreviewFrame, wxFrame )
	EVT_FILEPICKER_CHANGED( XRCID( "m_pickerTexture0" ), CRealtimePSPreviewFrame::OnFilePickerTexture0 )
	EVT_FILEPICKER_CHANGED( XRCID( "m_pickerTexture1" ), CRealtimePSPreviewFrame::OnFilePickerTexture1 )
	EVT_FILEPICKER_CHANGED( XRCID( "m_pickerTexture2" ), CRealtimePSPreviewFrame::OnFilePickerTexture2 )
	EVT_FILEPICKER_CHANGED( XRCID( "m_pickerTexture3" ), CRealtimePSPreviewFrame::OnFilePickerTexture3 )
	EVT_FILEPICKER_CHANGED( XRCID( "m_pickerTexture4" ), CRealtimePSPreviewFrame::OnFilePickerTexture4 )
	EVT_FILEPICKER_CHANGED( XRCID( "m_pickerTexture5" ), CRealtimePSPreviewFrame::OnFilePickerTexture5 )

	EVT_BUTTON( XRCID( "m_btnReset0" ), CRealtimePSPreviewFrame::OnResetTexture0 )
	EVT_BUTTON( XRCID( "m_btnReset1" ), CRealtimePSPreviewFrame::OnResetTexture1 )
	EVT_BUTTON( XRCID( "m_btnReset2" ), CRealtimePSPreviewFrame::OnResetTexture2 )
	EVT_BUTTON( XRCID( "m_btnReset3" ), CRealtimePSPreviewFrame::OnResetTexture3 )
	EVT_BUTTON( XRCID( "m_btnReset4" ), CRealtimePSPreviewFrame::OnResetTexture4 )
	EVT_BUTTON( XRCID( "m_btnReset5" ), CRealtimePSPreviewFrame::OnResetTexture5 )
	EVT_BUTTON( XRCID( "m_btnReset6" ), CRealtimePSPreviewFrame::OnResetTexture6 )
	EVT_BUTTON( XRCID( "m_btnResetAllTextures" ), CRealtimePSPreviewFrame::OnResetTextureAll )
END_EVENT_TABLE()

CRealtimePSPreviewFrame::CRealtimePSPreviewFrame( wxWindow* parent )
{
	wxXmlResource::Get()->LoadFrame( this, parent, "RealtimePSPreview" );

	// Width of rendering panel changes its size while options panel remains the same.
	wxSplitterWindow* pSplitter = XRCCTRL( *this, "Splitter", wxSplitterWindow );
	pSplitter->SetSashGravity(1.0);

	m_renderingPanel = (wxPanel*)pSplitter->GetWindow1();
	if (m_renderingPanel)
	{
		m_renderingPanel->Connect( wxEVT_SIZE, wxSizeEventHandler( CRealtimePSPreviewFrame::OnRenderingPanelSize ), nullptr, this );
		m_renderingPanel->Connect( wxEVT_IDLE, wxIdleEventHandler( CRealtimePSPreviewFrame::OnIdleEvent ), nullptr, this );
	}

	Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler( CRealtimePSPreviewFrame::OnCloseEvent), nullptr, this );
}

CRealtimePSPreviewFrame::~CRealtimePSPreviewFrame()
{
	m_driverD3D11.Cleanup();
}

void CRealtimePSPreviewFrame::InitD3D11()
{
	wxSplitterWindow* pSplitter = XRCCTRL( *this, "Splitter", wxSplitterWindow );
	wxPanel* renderingPanel = (wxPanel*)pSplitter->GetWindow1();

	bool bSuccess = m_driverD3D11.Initialize( renderingPanel->GetHWND(), renderingPanel->GetClientSize().GetWidth(), renderingPanel->GetClientSize().GetHeight() );
	if ( bSuccess )
	{
		// render the first time
		m_driverD3D11.Update();
		m_driverD3D11.Render();
	}
}

void CRealtimePSPreviewFrame::OnRenderingPanelSize( wxSizeEvent& evt )
{
	const int width = m_renderingPanel->GetClientSize().x;
	const int height = m_renderingPanel->GetClientSize().y;

	m_driverD3D11.ResizeViewport(width, height);

	evt.Skip();
}

void CRealtimePSPreviewFrame::OnIdleEvent( wxIdleEvent& evt )
{
	evt.RequestMore(true);

	m_driverD3D11.Update();
	m_driverD3D11.Render();
}

void CRealtimePSPreviewFrame::OnCloseEvent( wxCloseEvent& WXUNUSED(evt) )
{
	*m_bVisibility = false;

	Destroy();
}

// helper for getting filename from path.
wxString GetFilenameFromPath(const wxString& fullpath)
{
	const std::size_t posLastSlash = fullpath.find_last_of("\\");
	const wxString filename = fullpath.substr(posLastSlash+1);

	return filename;
}

//------------------------------------------------------------------------
void CRealtimePSPreviewFrame::UpdateUIForTexture( const wchar_t* path, unsigned int index )
{
	const bool bResult = m_driverD3D11.LoadTextureFromFile( path, index );
	if (bResult)
	{
		wxStaticText* pStatic = XRCCTRL(*this, wxString::Format("m_textTexture%d", index), wxStaticText);

		wxString texType;
		const ETextureType type = m_driverD3D11.GetTextureType( index );
		switch (type)
		{
			case ETextureType::ETexType_1D:			texType = "1D";			break;
			case ETextureType::ETexType_1DArray:	texType = "1DArr";		break;
			case ETextureType::ETexType_2D:			texType = "2D";			break;
			case ETextureType::ETexType_2DArray:	texType = "2DArr";		break;
			case ETextureType::ETexType_3D:			texType = "3D";			break;
			case ETextureType::ETexType_Cube:		texType = "Cube";		break;
			case ETextureType::ETexType_CubeArray:	texType = "CubeArr";	break;
		}

		pStatic->SetLabel( wxString::Format( "texture%d (%s, %s)", 
											 index,
											 texType,
											 GetFilenameFromPath( path ) ) );
	}
}

//------------------------------------------------------------------------
void CRealtimePSPreviewFrame::ResetUIForTexture( unsigned int index )
{
	wxStaticText* pStatic = XRCCTRL( *this, wxString::Format( "m_textTexture%d", index ), wxStaticText );
	pStatic->SetLabel( wxString::Format( "texture%d (null)", index ) );
}

//------------------------------------------------------------------------
void CRealtimePSPreviewFrame::OnFilePickerTexture0( wxFileDirPickerEvent& evt )
{
	const wxString path = evt.GetPath();
	
	UpdateUIForTexture( path.wc_str(), 0 );
}

//------------------------------------------------------------------------
void CRealtimePSPreviewFrame::OnFilePickerTexture1( wxFileDirPickerEvent& evt )
{
	const wxString path = evt.GetPath();

	UpdateUIForTexture( path.wc_str(), 1 );
}

//------------------------------------------------------------------------
void CRealtimePSPreviewFrame::OnFilePickerTexture2( wxFileDirPickerEvent& evt )
{
	const wxString path = evt.GetPath();

	UpdateUIForTexture( path.wc_str(), 2 );
}

//------------------------------------------------------------------------
void CRealtimePSPreviewFrame::OnFilePickerTexture3( wxFileDirPickerEvent& evt )
{
	const wxString path = evt.GetPath();

	UpdateUIForTexture( path.wc_str(), 3 );
}

void CRealtimePSPreviewFrame::OnFilePickerTexture4( wxFileDirPickerEvent& evt )
{
	const wxString path = evt.GetPath();

	UpdateUIForTexture( path.wc_str(), 4 );
}

void CRealtimePSPreviewFrame::OnFilePickerTexture5( wxFileDirPickerEvent& evt )
{
	const wxString path = evt.GetPath();

	UpdateUIForTexture( path.wc_str(), 5 );
}

void CRealtimePSPreviewFrame::OnResetTexture0( wxCommandEvent& WXUNUSED(evt) )
{
	m_driverD3D11.ResetTexture( 0 );
	ResetUIForTexture( 0 );
}

void CRealtimePSPreviewFrame::OnResetTexture1( wxCommandEvent& WXUNUSED( evt ) )
{
	m_driverD3D11.ResetTexture( 1 );
	ResetUIForTexture( 1 );
}

void CRealtimePSPreviewFrame::OnResetTexture2( wxCommandEvent& WXUNUSED( evt ) )
{
	m_driverD3D11.ResetTexture( 2 );
	ResetUIForTexture( 2 );
}

void CRealtimePSPreviewFrame::OnResetTexture3( wxCommandEvent& WXUNUSED( evt ) )
{
	m_driverD3D11.ResetTexture( 3 );
	ResetUIForTexture( 3 );
}

void CRealtimePSPreviewFrame::OnResetTexture4( wxCommandEvent& WXUNUSED( evt ) )
{
	m_driverD3D11.ResetTexture( 4 );
	ResetUIForTexture( 4 );
}

void CRealtimePSPreviewFrame::OnResetTexture5( wxCommandEvent& WXUNUSED( evt ) )
{
	m_driverD3D11.ResetTexture( 5 );
	ResetUIForTexture( 5 );
}

void CRealtimePSPreviewFrame::OnResetTexture6( wxCommandEvent& WXUNUSED( evt ) )
{
	m_driverD3D11.ResetTexture( 6 );
	ResetUIForTexture( 6 );
}

void CRealtimePSPreviewFrame::OnResetTextureAll( wxCommandEvent& WXUNUSED( evt ) )
{
	for ( int i=0; i < 6; i++ )
	{
		m_driverD3D11.ResetTexture( i );
		ResetUIForTexture( i );
	}
}