#include "PCH.h"
#include "CompilerModernDXLoader.h"

CModernD3DLoader::CModernD3DLoader()
{
	m_dxCompilerModule = nullptr;

	bool bSuccess = LoadD3DCompilerDLL( std::string("dxcompiler.dll") );
	if (!bSuccess)
	{
		// do something...	
	}
}

CModernD3DLoader::~CModernD3DLoader()
{
	::FreeLibrary( m_dxCompilerModule );
	m_dxCompilerModule = nullptr;
}

bool CModernD3DLoader::IsValid() const
{
	return (m_dxCompilerModule != nullptr);
}

bool CModernD3DLoader::LoadD3DCompilerDLL( const std::string& path )
{
	m_dxCompilerModule = ::LoadLibraryA( path.c_str() );

	if (m_dxCompilerModule == nullptr)
	{
		// costam

		return false;
	}

	return true;
}

