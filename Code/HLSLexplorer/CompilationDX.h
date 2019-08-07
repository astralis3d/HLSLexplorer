#pragma once

#include "Defines.h"

class CD3DCompilerLoader;

typedef std::vector<unsigned char> TByteBuffer;

namespace nmCompile
{
	std::string Compile( const SD3DOptions& options, const char* pData, const char* pEntrypoint, const char* pHLSLDirectory,
						 const CD3DCompilerLoader* pCompilerLoader, TByteBuffer& outDXBC );

	std::string CompileModern( const SD3DOptions& options, const char* pData, const wchar_t* pEntrypoint, 
							   const char* pHLSLDirectory, TByteBuffer& outBlob );

	// This is a simpler version which is used to compile only given shader and get Shader Model 6.0 blob
	void CompileModern_Simple( const char* pData, const wchar_t* pEntrypoint, const wchar_t* target, TByteBuffer& outBlob );
}

inline bool IsShaderProfile6(EShaderProfile shaderProfile)
{
	return (static_cast<int>(shaderProfile) >= static_cast<int>(EShaderProfile::ShaderProfile_6_0) );
}