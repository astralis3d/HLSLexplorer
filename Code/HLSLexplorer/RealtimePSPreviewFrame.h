#pragma once

class IRenderer;

class wxFileDirPickerEvent;

class CRealtimePSPreviewFrame : public wxFrame
{
public:
	CRealtimePSPreviewFrame(wxWindow* parent);
	virtual ~CRealtimePSPreviewFrame();

	void InitD3D11();
	void InitD3D12();

	void SetVisibilityPtr(bool* p) {  m_bVisibility = p;}

	//CDriverD3D11* GetD3D11Driver() { return &m_driverD3D11; } 
	IRenderer* GetRenderer() { return m_pRenderer; }

private:
	void OnRenderingPanelSize( wxSizeEvent& evt );
	void OnIdleEvent( wxIdleEvent& evt );
	void OnCloseEvent( wxCloseEvent& evt );

	void OnFilePickerTexture0( wxFileDirPickerEvent& evt );
	void OnFilePickerTexture1( wxFileDirPickerEvent& evt );
	void OnFilePickerTexture2( wxFileDirPickerEvent& evt );
	void OnFilePickerTexture3( wxFileDirPickerEvent& evt );
	void OnFilePickerTexture4( wxFileDirPickerEvent& evt );
	void OnFilePickerTexture5( wxFileDirPickerEvent& evt );

	void OnResetTexture0( wxCommandEvent& evt );
	void OnResetTexture1( wxCommandEvent& evt );
	void OnResetTexture2( wxCommandEvent& evt );
	void OnResetTexture3( wxCommandEvent& evt );
	void OnResetTexture4( wxCommandEvent& evt );
	void OnResetTexture5( wxCommandEvent& evt );
	void OnResetTexture6( wxCommandEvent& evt );
	void OnResetTextureAll( wxCommandEvent& evt );

	void UpdateUIForTexture( const wchar_t* path, unsigned int index);
	void ResetUIForTexture( unsigned int index );

	bool*	m_bVisibility;
	wxPanel* m_renderingPanel;

private:
	//CDriverD3D11 m_driverD3D11;
	IRenderer* m_pRenderer;

	DECLARE_EVENT_TABLE();
};