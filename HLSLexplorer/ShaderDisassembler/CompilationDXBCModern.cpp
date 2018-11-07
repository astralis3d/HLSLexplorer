#include "PCH.h"
#include "CompilationDXBC.h"
#include <d3dcompiler.h>
#include "official/dxcapi.h"
#include "CompilerLoader.h"

#pragma comment(lib, "official/dxcompiler.lib")

typedef std::vector<std::wstring> TParams;

// Modern DX compiler does not use flags, but fxc.exe switches instead.
// Details here:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb509709(v=vs.85).aspx

TParams GetCompileFlags( const SCompileFlags& compileFlags )
{
	TParams D3DCompileFlags;

	if (compileFlags.m_debug)							D3DCompileFlags.push_back( L"/Zi" );
	if (compileFlags.m_skipValidation)					D3DCompileFlags.push_back( L"/Vd" );
	if (compileFlags.m_skipOptimization)				D3DCompileFlags.push_back( L"/Od" );
	if (compileFlags.m_enableStrictness)				D3DCompileFlags.push_back( L"/Ges" );
	if (compileFlags.m_enableBackwardsCompatibility)	D3DCompileFlags.push_back( L"/Gec" );
	if (compileFlags.m_IEEEStrictness)					D3DCompileFlags.push_back( L"/Gis" );
	if (compileFlags.m_partialPrecision)				D3DCompileFlags.push_back( L"/Gpp" );
	if (compileFlags.m_WarningsAreErrors)				D3DCompileFlags.push_back( L"/WX" );
	if (compileFlags.m_ResourcesMayAlias)				D3DCompileFlags.push_back( L"/res_may_alias" );
	if (compileFlags.m_noPreshader)						D3DCompileFlags.push_back( L"/Op" );										// deprecated
	if (compileFlags.m_EnableUnboundedDescriptorTables)	D3DCompileFlags.push_back( L"/enable_unbounded_descriptor_tables" );		// DX12
	if (compileFlags.m_AllResourcesBound)				D3DCompileFlags.push_back( L"/all_resources_bound" );						// DX12

	//if (compileFlags.m_forceVSSoftwareNoOpt)			D3DCompileFlags |= D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT;		// couldn't find proper flag
	//if (compileFlags.m_forcePSSoftwareNoOpt)			D3DCompileFlags |= D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT;		// couldn't find proper flag

	switch (compileFlags.m_optimization)
	{
		case Opt_Level0:								D3DCompileFlags.push_back( L"/O0" );		break;
		case Opt_Level1:								D3DCompileFlags.push_back( L"/O1" );		break;
		case Opt_Level2:								D3DCompileFlags.push_back( L"/O2" );		break;
		case Opt_Level3:								D3DCompileFlags.push_back( L"/O3" );		break;
		default:	break;
	}

	switch (compileFlags.m_flowControl)
	{
		case FC_Avoid:									D3DCompileFlags.push_back( L"/Gfa" );	break;
		case FC_Prefer:									D3DCompileFlags.push_back( L"/Gfp" );	break;
		default:	break;
	}

	switch (compileFlags.m_packMatrix)
	{
		case PM_RowMajor:								D3DCompileFlags.push_back( L"/Zpr" );	break;
		case PM_ColumnMajor:							D3DCompileFlags.push_back( L"/Zpc" );	break;
		default:	break;
	}

	return D3DCompileFlags;
}

//------------------------------------------------------------------------
TParams GetDisassemblyFlags( const SDisassemblyFlags& flags )
{
	TParams D3DDisassemblyFlags;

	if (flags.m_enableColorCode)			D3DDisassemblyFlags.push_back( L"/Cc" );
	if (flags.m_enableInstructionNumbering)	D3DDisassemblyFlags.push_back( L"/Ni" );
	if (flags.m_enableInstructionOffset)	D3DDisassemblyFlags.push_back( L"/No" );
	if (flags.m_printHexLiterals)			D3DDisassemblyFlags.push_back( L"/Lx" );
	
	// if (flags.m_disableDebugInfo)			D3DDisassembleFlags |= D3D_DISASM_DISABLE_DEBUG_INFO;
	// if (flags.m_enableDefaultValuePrints)	D3DDisassembleFlags |= D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS;	// couldn't find proper switch
	// if (flags.m_instructionOnly)				D3DDisassembleFlags |= D3D_DISASM_INSTRUCTION_ONLY;				// couldn't find proper switch :(


	return D3DDisassemblyFlags;
}

//------------------------------------------------------------------------
TParams GetStripFlags( const SStrippingFlags& flags )
{
	TParams D3DStripFlags;

	if (flags.m_stripDebugInfo)				D3DStripFlags.push_back( L"/Qstrip_debug" );
	if (flags.m_stripReflectionData)		D3DStripFlags.push_back( L"/Qstrip_reflect" );
	if (flags.m_stripTestBlobs)				D3DStripFlags.push_back( L"/Qstrip_priv" );

	return D3DStripFlags;
}

std::wstring GetShaderTargetW( const SD3DOptions& options )
{
	// Determine shader target
	std::wstring shaderTarget;
	switch (options.shaderType)
	{
		case ShaderType_CS:	shaderTarget = std::wstring( L"cs_" );	break;
		case ShaderType_VS:	shaderTarget = std::wstring( L"vs_" );	break;
		case ShaderType_GS:	shaderTarget = std::wstring( L"gs_" );	break;
		case ShaderType_PS:	shaderTarget = std::wstring( L"ps_" );	break;
		case ShaderType_HS:	shaderTarget = std::wstring( L"hs_" );	break;
		case ShaderType_DS:	shaderTarget = std::wstring( L"ds_" );	break;
	}

	switch (options.shaderProfile)
	{
		case ShaderProfile_4_0:	shaderTarget += std::wstring( L"4_0" );	break;
		case ShaderProfile_5_0:	shaderTarget += std::wstring( L"5_0" );	break;
		case ShaderProfile_5_1:	shaderTarget += std::wstring( L"5_1" );	break;
		case ShaderProfile_6_0:	shaderTarget += std::wstring( L"6_0" );	break;
		case ShaderProfile_6_1:	shaderTarget += std::wstring( L"6_1" );	break;
		case ShaderProfile_6_2:	shaderTarget += std::wstring( L"6_2" );	break;
	}

	return shaderTarget;

}

std::string nmCompile::CompileModern( const SD3DOptions& options, const char* pData, const wchar_t* pEntrypoint, const char* pHLSLDirectory )
{
	// Modern DX shader compiler can perform only on Shader Model 6.0+
	const std::wstring shaderTarget = GetShaderTargetW( options );
	const size_t result = shaderTarget.find( L"s_6_" );
	if (result == std::string::npos)
	{
		return std::string( "Modern DX compiler is supported only on Shader Model 6.0+" );
	}


	// Before compilation, set current working directory (CWD) for that with opened HLSL shader
	TCHAR previousCWD[MAX_PATH];
	::GetCurrentDirectory( MAX_PATH, previousCWD );

	if (strlen( pHLSLDirectory ) > 0)
	{
		::SetCurrentDirectoryA( pHLSLDirectory );
	}



	// Based on:
	// https://blogs.msdn.microsoft.com/marcelolr/2017/03/27/directx-compiler-apis/

	IDxcLibrary* pLibrary = nullptr;
	IDxcBlobEncoding* pSource = nullptr;

	HRESULT hr = S_OK;
	hr = DxcCreateInstance( CLSID_DxcLibrary, __uuidof(IDxcLibrary), reinterpret_cast<void**>(&pLibrary) );
	if (FAILED( hr ))
	{
		return std::string( "DxcCreateInstance failed\n" );
	}

	hr = pLibrary->CreateBlobWithEncodingFromPinned( (LPBYTE)pData, strlen( pData ), CP_UTF8, &pSource );
	if (FAILED( hr ))
	{
		return std::string( "CreateBlobWithEncodingFromPinned failed\n" );
	}

	// Create compiler object
	IDxcCompiler* pCompiler = nullptr;
	hr = DxcCreateInstance( CLSID_DxcCompiler, __uuidof(IDxcCompiler), reinterpret_cast<void**>(&pCompiler) );
	if (FAILED( hr ))
	{
		return std::string( "DxcCreateInstance for IDxcCompiler failed\n" );
	}


	// params
	const TParams paramsCompile = GetCompileFlags( options.compileFlags );
	const TParams paramsDisassembly = GetDisassemblyFlags( options.disassemblyFlags );
	const TParams paramsStripping = GetStripFlags( options.strippingFlags );

	TParams paramsAll = paramsCompile;
	paramsAll.insert( paramsAll.end(), paramsDisassembly.begin(), paramsDisassembly.end() );
	paramsAll.insert( paramsAll.end(), paramsStripping.begin(), paramsStripping.end() );

	// We need to use c_str().
	std::vector<const wchar_t*> paramsAllCstr;
	for ( const auto& p : paramsAll )
	{
		paramsAllCstr.push_back( p.c_str() );
	}

	IDxcOperationResult* pResult = nullptr;
	hr = pCompiler->Compile( pSource,
						L"shader.hlsl",
						pEntrypoint,
						shaderTarget.c_str(),
						paramsAllCstr.data(),
						paramsAllCstr.size(),
						nullptr,
						0,
						nullptr,
						&pResult );
	
	// restore previous CWD
	::SetCurrentDirectory( previousCWD );



	std::string strOut;
	HRESULT hrCompilation = E_FAIL;
	if ( SUCCEEDED(hr) && pResult)
	{
		pResult->GetStatus( &hrCompilation );
	}	

	if (SUCCEEDED( hrCompilation ))
	{
		IDxcBlob* pBlob = nullptr;
		pResult->GetResult( &pBlob );

		// Disassemble now
		IDxcBlobEncoding* pDisassembled = nullptr;
		hr = pCompiler->Disassemble( pBlob, &pDisassembled );
		if (SUCCEEDED( hr ))
		{
			// Ok, shader succesfully disassembled.
			strOut = std::string( (const char*)pDisassembled->GetBufferPointer() );

			pDisassembled->Release();
		}
		else
		{
			return std::string( "IDxcCompiler::Disassemble failed\n" );
			//__debugbreak();
		}

		pBlob->Release();
		pResult->Release();
	}
	else
	{
		// Something went wrong.
		IDxcBlobEncoding *pPrintBlob, *pPrintBlob16;
		pResult->GetErrorBuffer( &pPrintBlob );
		// We can use the library to get our preferred encoding.
		pLibrary->GetBlobAsUtf16( pPrintBlob, &pPrintBlob16 );
		//wprintf( L"%*s", (int)pPrintBlob16->GetBufferSize() / 2, (LPCWSTR)pPrintBlob16->GetBufferPointer() );

		strOut = std::string( (const char*)pPrintBlob->GetBufferPointer() );

		pPrintBlob->Release();
		pPrintBlob16->Release();
	}

	pCompiler->Release();

	pSource->Release();
	pLibrary->Release();


	return strOut;
}
