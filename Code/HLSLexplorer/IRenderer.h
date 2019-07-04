#pragma once

struct SRendererCreateParams;

class IRenderer
{
public:
	virtual ~IRenderer() = 0 {}

	virtual bool Initialize(const SRendererCreateParams& createParams) = 0;
	virtual void Cleanup() = 0;

	virtual void Update() = 0;
	virtual void Render() = 0;

	virtual bool LoadTextureFromFile( const wchar_t* path, int index ) = 0;
	virtual void CreatePixelShader( const void* dxbcData, unsigned int size ) = 0;

	virtual ETextureType GetTextureType( int index ) const = 0;
	virtual void ResetTexture( int index ) = 0;

	virtual void ResizeViewport(unsigned int newWidth, unsigned int newHeight) = 0;
};