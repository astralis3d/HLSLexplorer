#ifndef __CONTROLSPANEL_H__
#define __CONTROLSPANEL_H__

#pragma once


#include <wx/wx.h>
#include "Defines.h"
#include "DisassemblerGCNISA.h"

class CControlsPanel : public wxPanel
{
public:
	CControlsPanel(wxWindow* parent, SD3DOptions* D3DOptions);

	E_ASIC_TYPE GetSelectedAsicType() const;

private:
	void OnSize(wxSizeEvent& event);

	// Shader Target
	void OnShaderTarget_Type(wxCommandEvent& evt);
	void OnShaderTarget_Profile(wxCommandEvent& evt);

	// Compile Flags
	void OnCompileFlag_Debug(wxCommandEvent& evt);
	void OnCompileFlag_SkipValidation(wxCommandEvent& evt);
	void OnCompileFlag_SkipOptimization(wxCommandEvent& evt);
	void OnCompileFlag_PartialPrecision(wxCommandEvent& evt);
	void OnCompileFlag_ForcsVSSoftwareNoOpt(wxCommandEvent& evt);
	void OnCompileFlag_ForcsPSSoftwareNoOpt(wxCommandEvent& evt);
	void OnCompileFlag_NoPreshader(wxCommandEvent& evt);
	void OnCompileFlag_EnableStrictness(wxCommandEvent& evt);
	void OnCompileFlag_EnableBackwardsCompatibility(wxCommandEvent& evt);
	void OnCompileFlag_IEEEStrictness(wxCommandEvent& evt);
	void OnCompileFlag_WarningsAreErrors(wxCommandEvent& evt);
	void OnCompileFlag_ResourcesMayAlias(wxCommandEvent& evt);
	void OnCompileFlag_EnableUnboundedDesciptorTables(wxCommandEvent& evt);
	void OnCompileFlag_AllResourcesBound(wxCommandEvent& evt);

	// Compile Flags #2
	void OnCompileFlag2_FlowControl(wxCommandEvent& evt);
	void OnCompileFlag2_PackMatrix(wxCommandEvent& evt);
	void OnCompileFlag2_Optimization(wxCommandEvent& evt);

	// Stripping
	void OnCheckboxStripping(wxCommandEvent& evt);

	void OnStrippingFlag_StripReflectionData(wxCommandEvent& evt);
	void OnStrippingFlag_StripDebugInfo(wxCommandEvent& evt);
	void OnStrippingFlag_StripTestBlobs(wxCommandEvent& evt);

	// Disassembly
	void OnDisassemblyFlag_EnableColorCode(wxCommandEvent& evt);
	void OnDisassemblyFlag_EnableDefaultValuePrints(wxCommandEvent& evt);
	void OnDisassemblyFlag_EnableInstructionNumbering(wxCommandEvent& evt);
	void OnDisassemblyFlag_DisableDebugInfo(wxCommandEvent& evt);
	void OnDisassemblyFlag_EnableInstructionOffset(wxCommandEvent& evt);
	void OnDisassemblyFlag_InstructionOnly(wxCommandEvent& evt);
	void OnDisassemblyFlag_PrintHexLiterals(wxCommandEvent& evt);

	// Others
	void OnCheckboxPerformD3DPreprocess(wxCommandEvent& evt);

	// Presets
	void OnButtonSavePreset(wxCommandEvent& evt);
	void OnButtonDeletePreset(wxCommandEvent& evt);
	void OnComboboxPresets(wxCommandEvent& evt);

	void OnChangeAsicChoice(wxCommandEvent& evt);

private:
	void InitializePresets();
	void UpdateControls();
	wxString GetFullPresetPath(const wxString& presetName) const;

	void UpdateCheckbox(const wxString& xrcName, bool value);
	void UpdateRadiobox(const wxString& xrcName, int val);

	SD3DOptions*		m_pOptions;

	E_ASIC_TYPE m_selectedAsic;

	wxDECLARE_EVENT_TABLE();
};

#endif