#pragma once

#include "RendererDefines.h"

class IRenderer;
class wxFileDirPickerEvent;

class CRealtimePSPreviewFrame : public wxFrame
{
public:
	CRealtimePSPreviewFrame(wxWindow* parent);
	virtual ~CRealtimePSPreviewFrame();

	void InitRenderer(ERendererAPI api);

	void SetVisibilityPtr(bool* p);

	IRenderer* GetRenderer();

private:
	void OnRenderingPanelSize( wxSizeEvent& evt );
	void OnIdleEvent( wxIdleEvent& evt );
	void OnCloseEvent( wxCloseEvent& evt );

	void UpdateWindowTitle();
	void OnMouseMotion( wxMouseEvent& evt );

	void OnToolbarButtonSave( wxCommandEvent& evt );

	void UpdateUIForTexture( const wchar_t* path, unsigned int index);
	void ResetUIForTexture( unsigned int index );

	bool*	m_bVisibility;
	wxPanel* m_renderingPanel;

private:
	//CDriverD3D11 m_driverD3D11;
	IRenderer* m_pRenderer;

	bool m_bIsRightDown;

	DECLARE_EVENT_TABLE();
};