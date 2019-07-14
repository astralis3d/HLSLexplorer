#include "PCH.h"
#include "RealtimePSPreviewFrame.h"

#include <wx/splitter.h>
#include <wx/filepicker.h>
#include <wx/xrc/xmlres.h>

#include "RendererD3D11.h"
#include "RendererD3D12.h"

struct SRendererCreateParams
{
	HWND hwnd;
	unsigned int width;
	unsigned int height;
};

CRealtimePSPreviewFrame::CRealtimePSPreviewFrame( wxWindow* parent )
	: m_pRenderer(nullptr)
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
		m_renderingPanel->Connect( wxEVT_MOTION, wxMouseEventHandler( CRealtimePSPreviewFrame::OnMouseMotion ), nullptr, this );
	}

	Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler( CRealtimePSPreviewFrame::OnCloseEvent), nullptr, this );

	for (int i = 0; i < 7; i++)
	{
		XRCCTRL(*this, wxString::Format("m_btnReset%d", i), wxButton)->Bind(wxEVT_BUTTON, [this, i](wxCommandEvent&)
		{
			m_pRenderer->ResetTexture( i );
			ResetUIForTexture( i );
		});

		XRCCTRL(*this, wxString::Format("m_pickerTexture%d", i), wxFilePickerCtrl)->Bind(wxEVT_FILEPICKER_CHANGED, [=](wxFileDirPickerEvent& evt)
		{
			const wxString path = evt.GetPath();
			UpdateUIForTexture( path.wc_str(), i );
		});
	}

	XRCCTRL( *this, "m_btnResetAllTextures", wxButton )->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
	{
		for ( int i=0; i < 7; i++ )
		{
			m_pRenderer->ResetTexture( i );
			ResetUIForTexture( i );
		}
	} );
}

CRealtimePSPreviewFrame::~CRealtimePSPreviewFrame()
{
	m_pRenderer->Cleanup();

	delete m_pRenderer;
	m_pRenderer = nullptr;
}

void CRealtimePSPreviewFrame::InitD3D11()
{
	wxSplitterWindow* pSplitter = XRCCTRL( *this, "Splitter", wxSplitterWindow );
	wxPanel* renderingPanel = (wxPanel*)pSplitter->GetWindow1();

	m_pRenderer = new CRendererD3D11();
	if (!m_pRenderer)
		return;

	SRendererCreateParams createParams;
	createParams.hwnd = renderingPanel->GetHWND();
	createParams.width = renderingPanel->GetClientSize().GetWidth();
	createParams.height = renderingPanel->GetClientSize().GetHeight();

	const bool bSuccess = m_pRenderer->Initialize( createParams );
	if ( bSuccess )
	{
		UpdateWindowTitle();

		// render the first time
		m_pRenderer->Update();
		m_pRenderer->Render();
	}
}

void CRealtimePSPreviewFrame::InitD3D12()
{
	wxSplitterWindow* pSplitter = XRCCTRL( *this, "Splitter", wxSplitterWindow );
	wxPanel* renderingPanel = (wxPanel*)pSplitter->GetWindow1();

	m_pRenderer = new CRendererD3D12();
	if (!m_pRenderer)
		return;

	SRendererCreateParams createParams;
	createParams.hwnd = renderingPanel->GetHWND();
	createParams.width = renderingPanel->GetClientSize().GetWidth();
	createParams.height = renderingPanel->GetClientSize().GetHeight();

	const bool bSuccess = m_pRenderer->Initialize( createParams );
	if (bSuccess)
	{
		UpdateWindowTitle();

		// render the first time
		m_pRenderer->Update();
		m_pRenderer->Render();
	}
}

//-----------------------------------------------------------------------------
void CRealtimePSPreviewFrame::SetVisibilityPtr( bool* p )
{
	m_bVisibility = p;
}

//-----------------------------------------------------------------------------
IRenderer* CRealtimePSPreviewFrame::GetRenderer()
{
	return m_pRenderer;
}

//-----------------------------------------------------------------------------
void CRealtimePSPreviewFrame::OnRenderingPanelSize( wxSizeEvent& evt )
{
	const int width = m_renderingPanel->GetClientSize().x;
	const int height = m_renderingPanel->GetClientSize().y;

	if (m_pRenderer)
	{
		m_pRenderer->ResizeViewport(width, height);
		UpdateWindowTitle();
	}

	evt.Skip();
}

//-----------------------------------------------------------------------------
void CRealtimePSPreviewFrame::OnIdleEvent( wxIdleEvent& evt )
{
	evt.RequestMore(true);

	m_pRenderer->Update();
	m_pRenderer->Render();
}

//-----------------------------------------------------------------------------
void CRealtimePSPreviewFrame::OnCloseEvent( wxCloseEvent& WXUNUSED(evt) )
{
	*m_bVisibility = false;

	Destroy();
}

//-----------------------------------------------------------------------------
void CRealtimePSPreviewFrame::UpdateWindowTitle()
{
	SetTitle( wxString::Format("Real-Time Pixel Shader Preview (%s) - %dx%d",
								GetRenderer()->GetRendererAPI() == RENDERER_API_D3D11 ? "D3D11" : "D3D12",
								m_renderingPanel->GetClientSize().x,
								m_renderingPanel->GetClientSize().y) );
}

//-----------------------------------------------------------------------------
void CRealtimePSPreviewFrame::OnMouseMotion( wxMouseEvent& evt )
{
	if (m_pRenderer)
	{
		m_pRenderer->SetCursorPosition( static_cast<unsigned int>(evt.GetX()), static_cast<unsigned int>(evt.GetY()) );
	}

	evt.Skip();
}

//-----------------------------------------------------------------------------
wxString GetFilenameFromPath(const wxString& fullpath)
{
	// helper for getting filename from path.
	const std::size_t posLastSlash = fullpath.find_last_of("\\");
	const wxString filename = fullpath.substr(posLastSlash+1);

	return filename;
}

//------------------------------------------------------------------------
void CRealtimePSPreviewFrame::UpdateUIForTexture( const wchar_t* path, unsigned int index )
{
	const bool bResult = m_pRenderer->LoadTextureFromFile( path, index );
	if (bResult)
	{
		wxStaticText* pStatic = XRCCTRL(*this, wxString::Format("m_textTexture%d", index), wxStaticText);

		wxString texType;
		const ETextureType type = m_pRenderer->GetTextureType( index );
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