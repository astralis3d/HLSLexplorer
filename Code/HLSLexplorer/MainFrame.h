#ifndef __MAINFRAME_H__
#define __MAINFRAME_H__

#pragma once

#include <wx/wx.h>
#include "Defines.h"
#include "CompilerLoader.h"
#include "CompilerModernDXLoader.h"
#include "DisassemblerGCNISA.h"
#include "RecentFilesManager.h"

// IDs for the controls and the menu commands
enum
{
	// it is important for the id corresponding to the "About" command to have
	// this standard value as otherwise it won't be handled properly under Mac
	// (where it is special and put into the "Apple" menu)
	ID_ABOUT = wxID_ABOUT,

	ID_FILE_EXIT = wxID_EXIT,


	ID_TOGGLEPANELVISIBILITY = wxID_HIGHEST + 1,
	ID_COMPILE,
	ID_REALTIME_PIXEL_SHADER_PREVIEW_D3D11,
	ID_REALTIME_PIXEL_SHADER_PREVIEW_D3D12,

	ID_LOAD_D3DCOMPILER_DLL,

	ID_OPEN_HLSL,

	ID_MENU_RECENT_FILES,
	ID_OPEN_RECENT_0,
	ID_OPEN_RECENT_1,
	ID_OPEN_RECENT_2,
	ID_OPEN_RECENT_3,
	ID_OPEN_RECENT_4,
	ID_OPEN_RECENT_5,
	ID_OPEN_RECENT_6,
	ID_OPEN_RECENT_7,
	ID_CLEAR_RECENT,

	ID_SAVE_AUTO,
	ID_SAVE_HLSL_SHADER,
	ID_SAVE_DISASSEMBLED_SHADER,

	ID_EDIT_GOTO_LINE,

	ID_INSERT_DUMMY_VERTEX_SHADER,
	ID_INSERT_DUMMY_PIXEL_SHADER,
	ID_INSERT_DUMMY_COMPUTE_SHADER,

	ID_SETTINGS_AUTO_WINDOW_SPLIT
};

// Forward decls
class CControlsPanel;
class CEditCtrl;
class wxSplitterWindow;
class wxNotebook;

class CRealtimePSPreviewFrame;

// Main frame
class CMyFrame : public wxFrame
{
public:
	CMyFrame( const wxString& str );

	void OnAbout( wxCommandEvent& event );
	void OnMenuFileExit( wxCommandEvent& event );

	// menu commands
	void OnMenuFileLoadD3DCompilerDLL( wxCommandEvent& evt );
	void OnMenuFileTogglePanelVisiblity( wxCommandEvent& evt );
	void OnMenuFileCompile( wxCommandEvent& evt );
	void OnMenuFileShowPSPreviewD3D11( wxCommandEvent& evt );
	void OnMenuFileShowPSPreviewD3D12( wxCommandEvent& evt );
	void OnMenuFileOpenHLSLShader( wxCommandEvent& evt );
	void OnMenuFileSaveAuto( wxCommandEvent& evt );
	void OnMenuFileSaveHLSLShader( wxCommandEvent& evt );
	void OnMenuFileSaveDisassembledShader( wxCommandEvent& evt );
	void OnMenuFileOpenRecent( wxCommandEvent& evt );
	void OnMenuFileClearRecent( wxCommandEvent& evt );

	void OnMenuEditGoToLine( wxCommandEvent& evt );

	void OnMenuInsertDummyVS( wxCommandEvent& evt );
	void OnMenuInsertDummyPS( wxCommandEvent& evt );
	void OnMenuInsertDummyCS( wxCommandEvent& evt );

	void OnMenuSettingsAutoWindowSplit( wxCommandEvent& evt );

	void OnCloseEvent( wxCloseEvent& evt );
	void OnSize( wxSizeEvent& event );

	void OnMouseLeftWindowActive( wxMouseEvent& evt );
	void OnMouseRightWindowDXBCActive( wxMouseEvent& evt );
	void OnMouseRightWindowDXILActive( wxMouseEvent& evt );
	void OnMouseRightWindowGCNISAActive( wxMouseEvent& evt );

private:
	void InitializeUI();
	void InitializeMenu();

	void UpdateRecentFiles();
	void ClearRecentFiles();
	std::string GetRecentFilesPath() const;

	void OpenShaderFile(const wxString& filepath);

	wxNotebook* CreateDisassemblerOutputNotebook( wxWindow* parent );

private:
	// main ui components
	wxPanel *			m_mainPanel = nullptr;
	CControlsPanel*		m_pControlsPanel;
	wxSplitterWindow*	m_pSplitter = nullptr;
	bool				m_bAutoWindowSplit = true;

	// left side (HLSL)
	CEditCtrl*		m_pEditHLSL;

	// right side (disassembled output)
	wxNotebook*		m_pRightWindow;
	CEditCtrl*		m_pEditASM_DXBC;
	CEditCtrl*		m_pEditASM_DXIL;
	CEditCtrl*		m_pEditASM_GCNISA;

	// Recent files
	CRecentFilesManager	m_recentFilesManager;
	wxMenu* m_submenuRecentFiles;


	// Compilation support
	CD3DCompilerLoader	m_compilerLoader;
	CModernD3DLoader	m_modernCompierLoader;
	CDisassemblerGCNISA	m_gcnisa;
	SD3DOptions			m_D3DOptions;

	// Pixel shader preview
	bool						m_bPSPreviewVisible = false;
	CRealtimePSPreviewFrame*	m_pPSPreviewFrame = nullptr;


	// Ctrl+S saves something different depending on which window 
	// is active (left click)
	bool			m_bHLSLWindowActive = true;
	bool			m_bASMWindowActive = false;
	CEditCtrl*		m_pEditSelected;

	wxString		m_strHLSLPath = wxEmptyString;

private:
	wxDECLARE_EVENT_TABLE();
};


#endif