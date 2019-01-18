#include "PCH.h"
#include "ControlsPanel.h"
#include "NewPresetDialog.h"

#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/xrc/xmlres.h>

BEGIN_EVENT_TABLE( CControlsPanel, wxPanel )

	EVT_SIZE( CControlsPanel::OnSize )
	EVT_RADIOBOX( XRCID( "radioboxShaderType" ), CControlsPanel::OnShaderTarget_Type )
	EVT_RADIOBOX( XRCID( "radioboxShaderProfile" ), CControlsPanel::OnShaderTarget_Profile )

	// Compile Flags
	EVT_CHECKBOX( XRCID( "optionCompileDebug" ), CControlsPanel::OnCompileFlag_Debug )
	EVT_CHECKBOX( XRCID( "optionCompileSkipValidation" ), CControlsPanel::OnCompileFlag_SkipValidation )
	EVT_CHECKBOX( XRCID( "optionCompileSkipOptimization" ), CControlsPanel::OnCompileFlag_SkipOptimization )
	EVT_CHECKBOX( XRCID( "optionCompilePartialPrecision" ), CControlsPanel::OnCompileFlag_PartialPrecision )
	EVT_CHECKBOX( XRCID( "optionCompileForceVSSoftwareNoOpt" ), CControlsPanel::OnCompileFlag_ForcsVSSoftwareNoOpt )
	EVT_CHECKBOX( XRCID( "optionCompileForcePSSoftNoOpt" ), CControlsPanel::OnCompileFlag_ForcsPSSoftwareNoOpt )
	EVT_CHECKBOX( XRCID( "optionCompileNoPreshader" ), CControlsPanel::OnCompileFlag_NoPreshader )
	EVT_CHECKBOX( XRCID( "optionCompileEnableStrictness" ), CControlsPanel::OnCompileFlag_EnableStrictness )
	EVT_CHECKBOX( XRCID( "optionCompileEnableBackwards" ), CControlsPanel::OnCompileFlag_EnableBackwardsCompatibility )
	EVT_CHECKBOX( XRCID( "optionCompileIEEEStrictness" ), CControlsPanel::OnCompileFlag_IEEEStrictness )
	EVT_CHECKBOX( XRCID( "optionCompileWarningsAreErrors" ), CControlsPanel::OnCompileFlag_WarningsAreErrors )
	EVT_CHECKBOX( XRCID( "optionCompileResourcesMayAlias" ), CControlsPanel::OnCompileFlag_ResourcesMayAlias )
	EVT_CHECKBOX( XRCID( "optionCompileEnableUnboundedDescriptorTables" ), CControlsPanel::OnCompileFlag_EnableUnboundedDesciptorTables )
	EVT_CHECKBOX( XRCID( "optionCompileAllResourcesBound" ), CControlsPanel::OnCompileFlag_AllResourcesBound )

	// Compile Flags #2
	EVT_RADIOBOX( XRCID( "radioboxFlowControl" ), CControlsPanel::OnCompileFlag2_FlowControl )
	EVT_RADIOBOX( XRCID( "radioboxPackMatrix" ), CControlsPanel::OnCompileFlag2_PackMatrix )
	EVT_RADIOBOX( XRCID( "radioboxOptimization" ), CControlsPanel::OnCompileFlag2_Optimization )

	// Stripping
	EVT_CHECKBOX( XRCID( "chPerformStripping" ), CControlsPanel::OnCheckboxStripping )
	EVT_CHECKBOX( XRCID( "optionStrippingReflectionData" ), CControlsPanel::OnStrippingFlag_StripReflectionData )
	EVT_CHECKBOX( XRCID( "optionStrippingDebugInfo" ), CControlsPanel::OnStrippingFlag_StripDebugInfo )
	EVT_CHECKBOX( XRCID( "optionStrippingTestBlobs" ), CControlsPanel::OnStrippingFlag_StripTestBlobs )

	// Disassembly
	EVT_CHECKBOX( XRCID( "optionDisassemblyEnableColorCodes" ), CControlsPanel::OnDisassemblyFlag_EnableColorCode )
	EVT_CHECKBOX( XRCID( "optionDisassemblyEnableDefaultValues" ), CControlsPanel::OnDisassemblyFlag_EnableDefaultValuePrints )
	EVT_CHECKBOX( XRCID( "optionDisassemblyEnableInstructionNumbering" ), CControlsPanel::OnDisassemblyFlag_EnableInstructionNumbering )
	EVT_CHECKBOX( XRCID( "optionDisassemblyDisableDebugInfo" ), CControlsPanel::OnDisassemblyFlag_DisableDebugInfo )
	EVT_CHECKBOX( XRCID( "optionDisassemblyEnableInstructionOffset" ), CControlsPanel::OnDisassemblyFlag_EnableInstructionOffset )
	EVT_CHECKBOX( XRCID( "optionDisassemblyInstructionOnly" ), CControlsPanel::OnDisassemblyFlag_InstructionOnly )
	EVT_CHECKBOX( XRCID( "optionDisassemblyPrintHexLiterals" ), CControlsPanel::OnDisassemblyFlag_PrintHexLiterals )

	// Others
	EVT_CHECKBOX( XRCID( "optionPreprocess" ), CControlsPanel::OnCheckboxPerformD3DPreprocess )

	// Presets
	EVT_COMBOBOX( XRCID( "comboboxPresets" ), CControlsPanel::OnComboboxPresets )
	EVT_BUTTON( XRCID( "btnSavePreset" ), CControlsPanel::OnButtonSavePreset )
	EVT_BUTTON( XRCID( "btnDeletePreset" ), CControlsPanel::OnButtonDeletePreset )

	EVT_CHOICE( XRCID("m_choiceAsic"), CControlsPanel::OnChangeAsicChoice )

END_EVENT_TABLE()


namespace
{
	wxArrayString GetAllPresetsNamesInDirectory()
	{
		// Get path to exe's directory
		const wxFileName f( wxStandardPaths::Get().GetExecutablePath() );
		const wxString appPath( f.GetPath() );

		// Find all files with *.preset extension
		wxArrayString presets;
		wxDir::GetAllFiles( appPath, &presets, wxT( "*.preset" ), wxDIR_FILES );

		// Append all names to list
		const size_t numPresets = presets.size();
		for (size_t i = 0; i < numPresets; ++i)
		{
			wxFileName preset( presets[i] );
			presets[i] = wxString( preset.GetName() );
		}

		return presets;
	}
}

//------------------------------------------------------------------------
CControlsPanel::CControlsPanel( wxWindow* parent, SD3DOptions* D3DOptions )
	: m_selectedAsic(E_ASIC_TYPE::AT_TAHITI)
{
	if (D3DOptions)
	{
		m_pOptions = D3DOptions;
	}

	wxXmlResource::Get()->LoadPanel( this, parent, wxT( "PanelControls" ) );

	// Set "5.0" default shader profile
	wxRadioBox* radioboxShaderProfile = XRCCTRL(*this, "radioboxShaderProfile", wxRadioBox);
	if (radioboxShaderProfile)
	{
		radioboxShaderProfile->SetSelection( (int) EShaderProfile::ShaderProfile_5_0 );
	}
	
	// Add ASIC types
	wxChoice* pAsicChoice = XRCCTRL(*this, "m_choiceAsic", wxChoice);
	if (pAsicChoice)
	{
		wxArrayString strings;
		strings.Add("Tahiti");
		strings.Add("Pitcairn");
		strings.Add("Capeverde");
		strings.Add("Oland");
		strings.Add("Hainan");
		strings.Add("Bonaire");
		strings.Add("Hawaii");
		strings.Add("Kalindi");
		strings.Add("Spectre");
		strings.Add("Spooky");
		strings.Add("Iceland");
		strings.Add("Tonga");
		strings.Add("Carrizo");
		strings.Add("Fiji");

		pAsicChoice->Append(strings);
		pAsicChoice->SetSelection(0);
	}

	InitializePresets();
}

//------------------------------------------------------------------------
void CControlsPanel::OnSize( wxSizeEvent& event )
{
	event.Skip();
}

//------------------------------------------------------------------------
void CControlsPanel::OnChangeAsicChoice( wxCommandEvent& evt )
{
	m_selectedAsic = static_cast<E_ASIC_TYPE>( evt.GetSelection() );
}

//------------------------------------------------------------------------
void CControlsPanel::OnShaderTarget_Type( wxCommandEvent& evt )
{
	m_pOptions->shaderType = (EShaderType)evt.GetSelection();
}

//------------------------------------------------------------------------
void CControlsPanel::OnShaderTarget_Profile( wxCommandEvent& evt )
{
	m_pOptions->shaderProfile = (EShaderProfile)evt.GetSelection();
}

//------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_Debug( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_debug = evt.IsChecked();
}

//------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_SkipValidation( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_skipValidation = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_SkipOptimization( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_skipOptimization = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_PartialPrecision( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_partialPrecision = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_ForcsVSSoftwareNoOpt( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_forceVSSoftwareNoOpt = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_ForcsPSSoftwareNoOpt( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_forcePSSoftwareNoOpt = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_NoPreshader( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_noPreshader = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_EnableStrictness( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_enableStrictness = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_EnableBackwardsCompatibility( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_enableBackwardsCompatibility = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_IEEEStrictness( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_IEEEStrictness = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_WarningsAreErrors( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_WarningsAreErrors = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_ResourcesMayAlias( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_ResourcesMayAlias = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_EnableUnboundedDesciptorTables( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_EnableUnboundedDescriptorTables = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag_AllResourcesBound( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_AllResourcesBound = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag2_FlowControl( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_flowControl = (EFlowControl)evt.GetSelection();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag2_PackMatrix( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_packMatrix = (EPackMatrix)evt.GetSelection();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCompileFlag2_Optimization( wxCommandEvent& evt )
{
	m_pOptions->compileFlags.m_optimization = (EOptimization)evt.GetSelection();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCheckboxStripping( wxCommandEvent& evt )
{
	m_pOptions->strippingFlags.m_EnableStripping = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnStrippingFlag_StripReflectionData( wxCommandEvent& evt )
{
	m_pOptions->strippingFlags.m_stripReflectionData = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnStrippingFlag_StripDebugInfo( wxCommandEvent& evt )
{
	m_pOptions->strippingFlags.m_stripDebugInfo = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnStrippingFlag_StripTestBlobs( wxCommandEvent& evt )
{
	m_pOptions->strippingFlags.m_stripTestBlobs = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnDisassemblyFlag_EnableColorCode( wxCommandEvent& evt )
{
	m_pOptions->disassemblyFlags.m_enableColorCode = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnDisassemblyFlag_EnableDefaultValuePrints( wxCommandEvent& evt )
{
	m_pOptions->disassemblyFlags.m_enableDefaultValuePrints = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnDisassemblyFlag_EnableInstructionNumbering( wxCommandEvent& evt )
{
	m_pOptions->disassemblyFlags.m_enableInstructionNumbering = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnDisassemblyFlag_DisableDebugInfo( wxCommandEvent& evt )
{
	m_pOptions->disassemblyFlags.m_disableDebugInfo = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnDisassemblyFlag_EnableInstructionOffset( wxCommandEvent& evt )
{
	m_pOptions->disassemblyFlags.m_enableInstructionOffset = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnDisassemblyFlag_InstructionOnly( wxCommandEvent& evt )
{
	m_pOptions->disassemblyFlags.m_instructionOnly = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnDisassemblyFlag_PrintHexLiterals( wxCommandEvent& evt )
{
	m_pOptions->disassemblyFlags.m_printHexLiterals = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnCheckboxPerformD3DPreprocess( wxCommandEvent& evt )
{
	m_pOptions->performPreprocess = evt.IsChecked();
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnButtonSavePreset( wxCommandEvent& WXUNUSED(evt) )
{
	CNewPresetDialog dialog( this->GetParent() );
	if (dialog.ShowModal() == wxID_OK)
	{
		// Create new file
		const wxString presetName = dialog.GetPresetName();
		const wxString presetPath = GetFullPresetPath( presetName );

		// Check if there is already preset named the same
		const wxArrayString presets = GetAllPresetsNamesInDirectory();

		bool bIsSameNamedPresetPresent = false;
		for (size_t i=0; i < presets.size(); i++)
		{
			if (presetName == presets[i])
			{
				bIsSameNamedPresetPresent = true;
				break;
			}
		}

		// Ask if there is already a preset named the same
		if (bIsSameNamedPresetPresent)
		{
			wxMessageDialog dlg( this,
								 wxString::Format("There is already a preset named %s. Do you want to replace it?", presetName),
								 "Question",
								 wxCENTER | wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION );

			const int answer = dlg.ShowModal();
			if (answer == wxID_NO)
			{
				return;
			}
		}

		// try to create file
		FILE* pFile = nullptr;
		fopen_s( &pFile, presetPath.c_str(), "wb" );

		if (!pFile)
		{
			wxMessageBox( wxT( "Unable to create file" ), wxT( "Error" ) );
			return;
		}
		else
		{
			fwrite( (const void*)&m_pOptions->compileFlags, sizeof( SCompileFlags ), 1, pFile );
			fwrite( (const void*)&m_pOptions->disassemblyFlags, sizeof( SDisassemblyFlags ), 1, pFile );
			fwrite( (const void*)&m_pOptions->strippingFlags, sizeof( SStrippingFlags ), 1, pFile );

			fclose( pFile );

			wxMessageBox( wxT( "File successfully created" ), wxT( "Information" ) );

			// Re-init
			InitializePresets();
		}
	}
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnButtonDeletePreset( wxCommandEvent& WXUNUSED(evt) )
{
	wxComboBox* pCombobox = XRCCTRL( *this, "comboboxPresets", wxComboBox );
	wxString presetName = pCombobox->GetStringSelection();
	
	if (!pCombobox)
		return;

	if (presetName == wxEmptyString)
		return;

	wxMessageDialog confirmDialog( this, wxString::Format("Are you sure you want to remove preset \"%s\"?", presetName), wxString("Confirm"), wxYES_NO | wxCENTER );
	if (confirmDialog.ShowModal() == wxID_YES)
	{	
		const wxString presetPath = GetFullPresetPath( presetName );

		// Remove file
		remove( presetPath.c_str() );

		// reinit all
		InitializePresets();
	}
}

//-----------------------------------------------------------------------------
void CControlsPanel::OnComboboxPresets( wxCommandEvent& WXUNUSED(evt) )
{
	wxComboBox* pCombobox = XRCCTRL( *this, "comboboxPresets", wxComboBox );
	if (!pCombobox)
		return;

	const wxString presetName = pCombobox->GetStringSelection();
	const wxString presetPath = GetFullPresetPath( presetName );

	// Open file
	FILE* pFile = nullptr;
	fopen_s( &pFile, presetPath.c_str(), "rb" );

	if (!pFile)
	{
		wxMessageBox( wxT( "Unable to open file" ), wxT( "Error" ) );
		return;
	}
	else
	{
		bool bSuccess = true;
		size_t count = 0;

		count = fread( (void*)&m_pOptions->compileFlags, sizeof( SCompileFlags ), 1, pFile );
		if (count != 1)
			bSuccess = false;

		count = fread( (void*)&m_pOptions->disassemblyFlags, sizeof( SDisassemblyFlags ), 1, pFile );
		if (count != 1)
			bSuccess = false;

		count = fread( (void*)&m_pOptions->strippingFlags, sizeof( SStrippingFlags ), 1, pFile );
		if (count != 1)
			bSuccess = false;

		fclose( pFile );

		if (bSuccess)
		{
			// update all controls
			UpdateControls();
		}
		else
		{
			wxMessageBox( wxT( "A problem with reading the preset occurred." ), wxT( "Error" ) );
			return;
		}
	}
}

//-----------------------------------------------------------------------------
E_ASIC_TYPE CControlsPanel::GetSelectedAsicType() const
{
	return m_selectedAsic;
}

//-----------------------------------------------------------------------------
void CControlsPanel::InitializePresets()
{
	const wxArrayString presets = GetAllPresetsNamesInDirectory();

	// Add presets to list
	wxComboBox* pCombobox = XRCCTRL( *this, "comboboxPresets", wxComboBox );
	pCombobox->Clear();
	pCombobox->Append( presets );

	// Update "Delete preset" button
	wxButton* btnDeletePreset = XRCCTRL(*this, "btnDeletePreset", wxButton);
	if (btnDeletePreset)
	{
		btnDeletePreset->Enable( !presets.empty() );
	}
}

//-----------------------------------------------------------------------------
void CControlsPanel::UpdateControls()
{
	// Update controls using new values from SD3DOptions.

	// Compile flags
	UpdateCheckbox( wxT( "optionCompileDebug" ), m_pOptions->compileFlags.m_debug );
	UpdateCheckbox( wxT( "optionCompileSkipValidation" ), m_pOptions->compileFlags.m_skipValidation );
	UpdateCheckbox( wxT( "optionCompileSkipOptimization" ), m_pOptions->compileFlags.m_skipOptimization );
	UpdateCheckbox( wxT( "optionCompilePartialPrecision" ), m_pOptions->compileFlags.m_partialPrecision );
	UpdateCheckbox( wxT( "optionCompileForceVSSoftwareNoOpt" ), m_pOptions->compileFlags.m_forceVSSoftwareNoOpt );
	UpdateCheckbox( wxT( "optionCompileForcePSSoftNoOpt" ), m_pOptions->compileFlags.m_forcePSSoftwareNoOpt );
	UpdateCheckbox( wxT( "optionCompileNoPreshader" ), m_pOptions->compileFlags.m_noPreshader );
	UpdateCheckbox( wxT( "optionCompileEnableStrictness" ), m_pOptions->compileFlags.m_enableStrictness );
	UpdateCheckbox( wxT( "optionCompileEnableBackwards" ), m_pOptions->compileFlags.m_enableBackwardsCompatibility );
	UpdateCheckbox( wxT( "optionCompileIEEEStrictness" ), m_pOptions->compileFlags.m_IEEEStrictness );
	UpdateCheckbox( wxT( "optionCompileWarningsAreErrors" ), m_pOptions->compileFlags.m_WarningsAreErrors );
	UpdateCheckbox( wxT( "optionCompileResourcesMayAlias" ), m_pOptions->compileFlags.m_ResourcesMayAlias );
	UpdateCheckbox( wxT( "optionCompileEnableUnboundedDescriptorTables" ), m_pOptions->compileFlags.m_EnableUnboundedDescriptorTables );
	UpdateCheckbox( wxT( "optionCompileAllResourcesBound" ), m_pOptions->compileFlags.m_AllResourcesBound );

	// Compile flags #2
	UpdateRadiobox( wxT( "radioboxFlowControl"), (int) m_pOptions->compileFlags.m_flowControl );
	UpdateRadiobox( wxT( "radioboxPackMatrix" ), (int)m_pOptions->compileFlags.m_packMatrix );
	UpdateRadiobox( wxT( "radioboxOptimization" ), (int)m_pOptions->compileFlags.m_optimization );

	// Stripping
	UpdateCheckbox( wxT( "chPerformStripping" ), m_pOptions->strippingFlags.m_EnableStripping );
	UpdateCheckbox( wxT( "optionStrippingReflectionData" ), m_pOptions->strippingFlags.m_stripReflectionData );
	UpdateCheckbox( wxT( "optionStrippingDebugInfo" ), m_pOptions->strippingFlags.m_stripDebugInfo );
	UpdateCheckbox( wxT( "optionStrippingTestBlobs" ), m_pOptions->strippingFlags.m_stripTestBlobs );

	// Disassembly
	UpdateCheckbox( wxT( "optionDisassemblyEnableColorCodes"), m_pOptions->disassemblyFlags.m_enableColorCode );
	UpdateCheckbox( wxT( "optionDisassemblyEnableDefaultValues" ), m_pOptions->disassemblyFlags.m_enableDefaultValuePrints );
	UpdateCheckbox( wxT( "optionDisassemblyEnableInstructionNumbering" ), m_pOptions->disassemblyFlags.m_enableInstructionNumbering );
	UpdateCheckbox( wxT( "optionDisassemblyDisableDebugInfo" ), m_pOptions->disassemblyFlags.m_disableDebugInfo );
	UpdateCheckbox( wxT( "optionDisassemblyEnableInstructionOffset" ), m_pOptions->disassemblyFlags.m_enableInstructionOffset );
	UpdateCheckbox( wxT( "optionDisassemblyInstructionOnly" ), m_pOptions->disassemblyFlags.m_instructionOnly );
	UpdateCheckbox( wxT( "optionDisassemblyPrintHexLiterals" ), m_pOptions->disassemblyFlags.m_printHexLiterals );

	// Others
	UpdateCheckbox( wxT( "optionPreprocess"), m_pOptions->performPreprocess );
}

//-----------------------------------------------------------------------------
wxString CControlsPanel::GetFullPresetPath( const wxString& presetName ) const
{
	// Get path to full file
	const wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	const size_t x = exePath.find_last_of( "\\/" );
	wxString presetPath = exePath.substr( 0, x + 1 );
	presetPath.append( presetName );
	presetPath.append( wxT( ".preset" ) );

	return presetPath;
}

//-----------------------------------------------------------------------------
void CControlsPanel::UpdateCheckbox( const wxString& xrcName, bool value )
{
	wxCheckBox* pCheckbox = XRCCTRL( *this, xrcName, wxCheckBox );
	if (pCheckbox)
	{
		pCheckbox->SetValue( value );
	}
}

//-----------------------------------------------------------------------------
void CControlsPanel::UpdateRadiobox( const wxString& xrcName, int val )
{
	wxRadioBox* pRadioBox = XRCCTRL( *this, xrcName, wxRadioBox );
	if (pRadioBox)
	{
		pRadioBox->SetSelection( val );
	}
}

