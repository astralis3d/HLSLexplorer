#include "PCH.h"
#include "CompilerLoader.h"

CD3DCompilerLoader::CD3DCompilerLoader()
	: m_pD3DCompile(nullptr)
	, m_pD3DDisassemble(nullptr)
	, m_pD3DPreprocess(nullptr)
	, m_pD3DStripShader(nullptr)
	, m_D3DCompilerModule(nullptr)
{
	LoadDefaultCompiler();
}

//------------------------------------------------------------------------
CD3DCompilerLoader::~CD3DCompilerLoader()
{
	::FreeLibrary(m_D3DCompilerModule);
	m_D3DCompilerModule = nullptr;
}

//------------------------------------------------------------------------
bool CD3DCompilerLoader::LoadD3DCompilerDLL( const std::string& path )
{
	m_D3DCompilerModule = ::LoadLibraryA( path.c_str() );
	if (m_D3DCompilerModule)
	{
		m_pD3DCompile = (pD3DCompile) ::GetProcAddress(m_D3DCompilerModule, "D3DCompile");
		m_pD3DDisassemble = (pD3DDisassemble) ::GetProcAddress(m_D3DCompilerModule, "D3DDisassemble");
		m_pD3DPreprocess = (pD3DPreprocess) ::GetProcAddress(m_D3DCompilerModule, "D3DPreprocess");
		m_pD3DStripShader = (pD3DStripShader) ::GetProcAddress(m_D3DCompilerModule, "D3DStripShader");

		if ( IsValid() )
		{
			// ReloadInfoVersion();
			return true;
		}
		else
		{
			m_pD3DCompile = nullptr;
			m_pD3DDisassemble = nullptr;
			m_pD3DPreprocess = nullptr;
			m_pD3DStripShader = nullptr;

			m_D3DCompilerModule = nullptr;

			// Try to restore default D3DCompiler
			LoadDefaultCompiler();

			return false;
		}
	}
	else
	{
		m_pD3DCompile = nullptr;
		m_pD3DDisassemble = nullptr;
		m_pD3DPreprocess = nullptr;
		m_pD3DStripShader = nullptr;

		m_D3DCompilerModule = nullptr;

		return false;
	}
}

//------------------------------------------------------------------------
bool CD3DCompilerLoader::LoadDefaultCompiler()
{
	// Attempt 1 - from CWD
	bool bResult = LoadD3DCompilerDLL( std::string( "d3dcompiler_47.dll" ) );

	// Attempt 2 if failed - from windir. Consider both 64-bit and 32-bit cases
	if (!bResult)
	{
		std::string finalPath;

		char str[MAX_PATH];
		::GetSystemDirectoryA( str, sizeof( str ) );
		finalPath = std::string( str );
		finalPath += "\\D3DCompiler_47.dll";

		bResult = LoadD3DCompilerDLL( finalPath );
	}

	return bResult;
}

//------------------------------------------------------------------------
bool CD3DCompilerLoader::IsValid() const
{
	return(	m_pD3DCompile != nullptr &&
			m_pD3DDisassemble != nullptr &&
			m_pD3DPreprocess != nullptr &&
			m_pD3DStripShader != nullptr);
}
