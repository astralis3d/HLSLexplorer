#ifndef __DEFINES_H__
#define __DEFINES_H__

#pragma once

enum EShaderType
{
	ShaderType_VS = 0,
	ShaderType_GS,
	ShaderType_PS,
	ShaderType_HS,
	ShaderType_DS,
	ShaderType_CS
};

enum EShaderProfile
{
	ShaderProfile_4_0 = 0,
	ShaderProfile_4_1,
	ShaderProfile_5_0,
	ShaderProfile_5_1,
	ShaderProfile_6_0,
	ShaderProfile_6_1,
	ShaderProfile_6_2,
	ShaderProfile_6_3
};

enum EOptimization
{
	Opt_NotSet = 0,
	Opt_Level0,
	Opt_Level1,
	Opt_Level2,
	Opt_Level3
};

enum EFlowControl
{
	FC_NotSet = 0,
	FC_Avoid,
	FC_Prefer
};

enum EPackMatrix
{
	PM_NotSet = 0,
	PM_RowMajor,
	PM_ColumnMajor
};

struct SStrippingFlags
{
	SStrippingFlags()
		: m_EnableStripping(false)
		, m_stripReflectionData(false)
		, m_stripDebugInfo(false)
		, m_stripTestBlobs(false)
	{
	}

	bool m_EnableStripping;

	bool m_stripReflectionData;
	bool m_stripDebugInfo;
	bool m_stripTestBlobs;
};

struct SDisassemblyFlags
{
	SDisassemblyFlags()
		: m_enableColorCode(false)
		, m_enableDefaultValuePrints(false)
		, m_enableInstructionNumbering(false)
		, m_disableDebugInfo(false)
		, m_enableInstructionOffset(false)
		, m_instructionOnly(false)
		, m_printHexLiterals(false)
	{
	}
	
	bool m_enableColorCode;
	bool m_enableDefaultValuePrints;
	bool m_enableInstructionNumbering;
	bool m_disableDebugInfo;
	bool m_enableInstructionOffset;
	bool m_instructionOnly;
	bool m_printHexLiterals;
};

struct SCompileFlags
{
	SCompileFlags()
		: m_debug(false)
		, m_skipValidation(false)
		, m_skipOptimization(false)
		, m_partialPrecision(false)
		, m_forceVSSoftwareNoOpt(false)
		, m_forcePSSoftwareNoOpt(false)
		, m_noPreshader(false)
		, m_enableStrictness(false)
		, m_enableBackwardsCompatibility(false)
		, m_IEEEStrictness(false)
		, m_WarningsAreErrors(false)
		, m_ResourcesMayAlias(false)
		, m_AllResourcesBound(false)
		, m_optimization(EOptimization::Opt_NotSet)
		, m_flowControl(EFlowControl::FC_NotSet)
		, m_packMatrix(EPackMatrix::PM_NotSet)
	{
	}


	bool m_debug;
	bool m_skipValidation;
	bool m_skipOptimization;
	bool m_partialPrecision;
	bool m_forceVSSoftwareNoOpt;
	bool m_forcePSSoftwareNoOpt;
	bool m_noPreshader;
	bool m_enableStrictness;
	bool m_enableBackwardsCompatibility;
	bool m_IEEEStrictness;
	bool m_WarningsAreErrors;
	bool m_ResourcesMayAlias;
	bool m_EnableUnboundedDescriptorTables;
	bool m_AllResourcesBound;

	EOptimization m_optimization;
	EFlowControl m_flowControl;
	EPackMatrix m_packMatrix;
};


struct SD3DOptions
{
	SD3DOptions()
		: compileFlags()
		, disassemblyFlags()
		, strippingFlags()
		, performPreprocess(false)
		, shaderType(EShaderType::ShaderType_VS)
		, shaderProfile(EShaderProfile::ShaderProfile_5_0)
	{
	}
	
	SCompileFlags		compileFlags;
	SDisassemblyFlags	disassemblyFlags;
	SStrippingFlags		strippingFlags;

	bool				performPreprocess;

	// used to determine target
	EShaderType			shaderType;
	EShaderProfile		shaderProfile;
};


#endif