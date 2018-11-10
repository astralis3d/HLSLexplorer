#pragma once

#include <string>
#include <d3dcompiler.h>

typedef HRESULT (WINAPI * pD3DStripShader)(LPCVOID, SIZE_T, UINT, ID3DBlob**);

class CD3DCompilerLoader
{
public:
	CD3DCompilerLoader();	// default constructor
	~CD3DCompilerLoader();

	bool IsValid() const;
	bool LoadD3DCompilerDLL( const std::string& path );

private:
	bool LoadDefaultCompiler();

public:
	pD3DCompile			m_pD3DCompile;
	pD3DPreprocess		m_pD3DPreprocess;
	pD3DStripShader		m_pD3DStripShader;
	pD3DDisassemble		m_pD3DDisassemble;

private:
	HMODULE m_D3DCompilerModule;
};