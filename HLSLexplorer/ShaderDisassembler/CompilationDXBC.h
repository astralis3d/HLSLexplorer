#pragma once

#include "Defines.h"

class CD3DCompilerLoader;

typedef std::vector<unsigned char> TByteBuffer;

namespace nmCompile
{
	std::string Compile( const SD3DOptions& options, const char* pData, const char* pEntrypoint, const char* pHLSLDirectory,
						 const CD3DCompilerLoader* pCompilerLoader, TByteBuffer& outDXBC );

	std::string CompileModern( const SD3DOptions& options, const char* pData, const wchar_t* pEntrypoint, 
							   const char* pHLSLDirectory );
}