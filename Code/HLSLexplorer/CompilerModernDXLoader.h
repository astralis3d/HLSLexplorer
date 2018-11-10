#pragma once

#include <string>

class CModernD3DLoader
{
public:
	CModernD3DLoader();	// default constructor
	~CModernD3DLoader();

	bool IsValid() const;
	bool LoadD3DCompilerDLL( const std::string& path );

private:
	HMODULE		m_dxCompilerModule;
};