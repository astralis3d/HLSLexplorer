#pragma once

#include "Defines.h"

struct SRendererCreateParams;

enum ERendererAPI
{
	RENDERER_API_D3D11,
	RENDERER_API_D3D12
};

class IRenderer
{
public:
	virtual ~IRenderer() {}

	virtual bool Initialize(const SRendererCreateParams& createParams) = 0;
	virtual void Cleanup() = 0;

	virtual void Update() = 0;
	virtual void Render() = 0;

	virtual ERendererAPI GetRendererAPI() const = 0;

	virtual bool LoadTextureFromFile( const wchar_t* path, int index ) = 0;
	virtual void UpdatePixelShader( const void* dxbcData, unsigned int size, EShaderProfile shaderProfile ) = 0;

	virtual ETextureType GetTextureType( int index ) const = 0;
	virtual void ResetTexture( int index ) = 0;

	virtual void SetCursorPosition( unsigned int x, unsigned int y ) = 0;
	virtual void ResizeViewport(unsigned int newWidth, unsigned int newHeight) = 0;

	virtual Vec4 GetColorAtCursorPosition(unsigned int& x, unsigned int& y) const = 0;
};