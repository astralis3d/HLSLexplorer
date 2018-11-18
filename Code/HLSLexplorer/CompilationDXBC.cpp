#include "PCH.h"
#include "CompilationDX.h"
#include <d3dcompiler.h>
#include "official/dxcapi.h"
#include "CompilerLoader.h"

UINT GetCompileFlags( const SCompileFlags& compileFlags )
{
	UINT D3DCompileFlags = 0;

	if (compileFlags.m_debug)							D3DCompileFlags |= D3DCOMPILE_DEBUG;
	if (compileFlags.m_skipValidation)					D3DCompileFlags |= D3DCOMPILE_SKIP_VALIDATION;
	if (compileFlags.m_skipOptimization)				D3DCompileFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	if (compileFlags.m_enableBackwardsCompatibility)	D3DCompileFlags |= D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
	if (compileFlags.m_forceVSSoftwareNoOpt)			D3DCompileFlags |= D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT;
	if (compileFlags.m_forcePSSoftwareNoOpt)			D3DCompileFlags |= D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT;
	if (compileFlags.m_noPreshader)						D3DCompileFlags |= D3DCOMPILE_NO_PRESHADER;
	if (compileFlags.m_enableStrictness)				D3DCompileFlags |= D3DCOMPILE_ENABLE_STRICTNESS;
	if (compileFlags.m_IEEEStrictness)					D3DCompileFlags |= D3DCOMPILE_IEEE_STRICTNESS;
	if (compileFlags.m_partialPrecision)				D3DCompileFlags |= D3DCOMPILE_PARTIAL_PRECISION;
	if (compileFlags.m_WarningsAreErrors)				D3DCompileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
	if (compileFlags.m_ResourcesMayAlias)				D3DCompileFlags |= D3DCOMPILE_RESOURCES_MAY_ALIAS;
	if (compileFlags.m_EnableUnboundedDescriptorTables)	D3DCompileFlags |= D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
	if (compileFlags.m_AllResourcesBound)				D3DCompileFlags |= D3DCOMPILE_ALL_RESOURCES_BOUND;

	switch (compileFlags.m_optimization)
	{
		case Opt_Level0:								D3DCompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL0;	break;
		case Opt_Level1:								D3DCompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;	break;
		case Opt_Level2:								D3DCompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL2;	break;
		case Opt_Level3:								D3DCompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;	break;
		default:	break;
	}

	switch (compileFlags.m_flowControl)
	{
		case FC_Avoid:									D3DCompileFlags |= D3DCOMPILE_AVOID_FLOW_CONTROL;	break;
		case FC_Prefer:									D3DCompileFlags |= D3DCOMPILE_PREFER_FLOW_CONTROL;	break;
		default:	break;
	}

	switch (compileFlags.m_packMatrix)
	{
		case PM_RowMajor:								D3DCompileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;	break;
		case PM_ColumnMajor:							D3DCompileFlags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;	break;
		default:	break;
	}

	return D3DCompileFlags;
}

//------------------------------------------------------------------------
UINT GetDisassemblyFlags( const SDisassemblyFlags& flags )
{
	UINT D3DDisassembleFlags = 0;

	if (flags.m_enableColorCode)			D3DDisassembleFlags |= D3D_DISASM_ENABLE_COLOR_CODE;
	if (flags.m_enableDefaultValuePrints)	D3DDisassembleFlags |= D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS;
	if (flags.m_enableInstructionNumbering)	D3DDisassembleFlags |= D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING;
	if (flags.m_disableDebugInfo)			D3DDisassembleFlags |= D3D_DISASM_DISABLE_DEBUG_INFO;
	if (flags.m_enableInstructionOffset)	D3DDisassembleFlags |= D3D_DISASM_ENABLE_INSTRUCTION_OFFSET;
	if (flags.m_instructionOnly)			D3DDisassembleFlags |= D3D_DISASM_INSTRUCTION_ONLY;
	if (flags.m_printHexLiterals)			D3DDisassembleFlags |= D3D_DISASM_PRINT_HEX_LITERALS;

	return D3DDisassembleFlags;
}

//------------------------------------------------------------------------
UINT GetStripFlags( const SStrippingFlags& flags )
{
	UINT D3DStripFlags = 0;

	if (flags.m_stripDebugInfo)				D3DStripFlags |= D3DCOMPILER_STRIP_DEBUG_INFO;
	if (flags.m_stripReflectionData)		D3DStripFlags |= D3DCOMPILER_STRIP_REFLECTION_DATA;
	if (flags.m_stripTestBlobs)				D3DStripFlags |= D3DCOMPILER_STRIP_TEST_BLOBS;

	return D3DStripFlags;
}

std::string GetShaderTargetA( const SD3DOptions& options )
{
	// Determine shader target
	std::string shaderTarget;
	switch (options.shaderType)
	{
		case ShaderType_CS:	shaderTarget = std::string( "cs_" );	break;
		case ShaderType_VS:	shaderTarget = std::string( "vs_" );	break;
		case ShaderType_GS:	shaderTarget = std::string( "gs_" );	break;
		case ShaderType_PS:	shaderTarget = std::string( "ps_" );	break;
		case ShaderType_HS:	shaderTarget = std::string( "hs_" );	break;
		case ShaderType_DS:	shaderTarget = std::string( "ds_" );	break;
	}

	switch (options.shaderProfile)
	{
		case ShaderProfile_4_0:	shaderTarget += std::string( "4_0" );	break;
		case ShaderProfile_5_0:	shaderTarget += std::string( "5_0" );	break;
		case ShaderProfile_5_1:	shaderTarget += std::string( "5_1" );	break;
		case ShaderProfile_6_0:	shaderTarget += std::string( "6_0" );	break;
		case ShaderProfile_6_1:	shaderTarget += std::string( "6_1" );	break;
		case ShaderProfile_6_2:	shaderTarget += std::string( "6_2" );	break;
		case ShaderProfile_6_3:	shaderTarget += std::string( "6_3" );	break;
	}

	return shaderTarget;

}

//------------------------------------------------------------------------
std::string nmCompile::Compile( const SD3DOptions& options, const char* pData, const char* pEntrypoint, const char* pHLSLDirectory, const CD3DCompilerLoader* pCompilerLoader, TByteBuffer& outDXBC )
{
	if (!pData)
		return std::string( "ERROR: Invalid data ptr" );

	auto lpfnD3DCompile = *pCompilerLoader->m_pD3DCompile;
	auto lpfnD3DPreprocess = *pCompilerLoader->m_pD3DPreprocess;
	auto lpfnD3DStripShader = *pCompilerLoader->m_pD3DStripShader;
	auto lpfnD3DDisassemble = *pCompilerLoader->m_pD3DDisassemble;


	// Input shader code length
	const size_t dataLen = strlen( pData );

	const std::string shaderTarget = GetShaderTargetA(options);
	const SCompileFlags& compileFlags = options.compileFlags;
	const SStrippingFlags& strippingFlags = options.strippingFlags;
	const SDisassemblyFlags& disassemblyFlags = options.disassemblyFlags;

	// Determine compile flags
	const UINT D3DCompileFlags = GetCompileFlags( compileFlags );


	HRESULT hr = E_FAIL;
	ID3DBlob* pShaderBlob = nullptr;
	ID3DBlob* pMessagesBlob = nullptr;
	
	ID3DBlob* pBlobTextPreprocess = nullptr;
	bool bUsePreprocessedBlob = false;
	
	// Preprocess shader if user wants to.
	if (options.performPreprocess)
	{
		hr = lpfnD3DPreprocess( (const void*)pData,
								dataLen,
								nullptr,
								nullptr,
								D3D_COMPILE_STANDARD_FILE_INCLUDE,
								&pBlobTextPreprocess,
								&pMessagesBlob );

		bUsePreprocessedBlob = SUCCEEDED(hr);

		if (FAILED( hr ))
		{
			// An error during preprocessing occured.
			const char* pMessages = (const char*)pMessagesBlob->GetBufferPointer();
			std::string strMessages = std::string( pMessages );

			pMessagesBlob->Release();


			return strMessages;
		}
	}


	// Before compilation, set current working directory (CWD) for that with opened HLSL shader
	TCHAR previousCWD[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, previousCWD);

	if ( strlen(pHLSLDirectory) > 0 )
	{
		::SetCurrentDirectoryA( pHLSLDirectory );
	}

	// Perform compilation
	hr = lpfnD3DCompile( (bUsePreprocessedBlob) ? (pBlobTextPreprocess->GetBufferPointer()) : (const void*) pData,
						 (bUsePreprocessedBlob) ? (pBlobTextPreprocess->GetBufferSize()) : dataLen,
						 nullptr,
						 nullptr,
						 D3D_COMPILE_STANDARD_FILE_INCLUDE,
						 pEntrypoint,
						 shaderTarget.c_str(),
						 D3DCompileFlags,
						 0,
						 &pShaderBlob,
						 &pMessagesBlob );

	// Restore previous CWD
	::SetCurrentDirectory(previousCWD);

	// We want to keep compiled DXBC.
	if (SUCCEEDED(hr))
	{
		outDXBC.clear();
		outDXBC.resize( pShaderBlob->GetBufferSize() );

		memcpy( outDXBC.data(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize() );
	}

	SAFE_RELEASE(pBlobTextPreprocess);

	if (SUCCEEDED( hr ))
	{
		// Ok, shader successfully compiled.
		// Strip shader now if user wants to.

		ID3DBlob* pStrippedBlob = nullptr;
		if (strippingFlags.m_EnableStripping)
		{
			const UINT D3DStripFlags = GetStripFlags( strippingFlags );
			hr = lpfnD3DStripShader( pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), D3DStripFlags, &pStrippedBlob );

			if (FAILED( hr ))
			{
				if (pShaderBlob)
					pShaderBlob->Release();

				return std::string( "D3DStripShader failed\n" );
			}
		}


		// DISASSEMBLE SHADER
		ID3DBlob* pBlobDisassembled = nullptr;

		hr = lpfnD3DDisassemble( (strippingFlags.m_EnableStripping) ? pStrippedBlob->GetBufferPointer() : pShaderBlob->GetBufferPointer(),
			(strippingFlags.m_EnableStripping) ? pStrippedBlob->GetBufferSize() : pShaderBlob->GetBufferSize(),
							 GetDisassemblyFlags( disassemblyFlags ),
							 nullptr,
							 &pBlobDisassembled );

		SAFE_RELEASE(pStrippedBlob);

		if (SUCCEEDED( hr ))
		{
			// Ok, shader succesfully disassembled.
			const std::string strDisassembled = std::string( (const char*)pBlobDisassembled->GetBufferPointer() );
			pBlobDisassembled->Release();

			return strDisassembled;
		}
		else
		{
			return std::string( "ERROR: D3DDisassemble failed\n" );
		}
	}
	else
	{
		// An error during compiling occurred, return error message.
		const char* pMessages = (const char*) pMessagesBlob->GetBufferPointer();
		const std::string strMessages = std::string( pMessages );

		pMessagesBlob->Release();
	
		return strMessages;
	}
}

