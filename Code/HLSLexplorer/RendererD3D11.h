#pragma once

// Basic D3D11 component
#include <d3d11.h>
#include "Defines.h"
#include "Renderer.h"

struct SRendererCreateParams;

class CRendererD3D11 : public CRenderer
{
public:
	CRendererD3D11();
	virtual ~CRendererD3D11() {}

	virtual bool Initialize(const SRendererCreateParams& createParams);
	virtual void Cleanup();

	virtual void Update();
	virtual void Render();

	virtual ERendererAPI GetRendererAPI() const;

	virtual bool LoadTextureFromFile(const wchar_t* path, int index);
	virtual void UpdatePixelShader(const void* dxbcData, unsigned int size, EShaderProfile shaderProfile);
	
	virtual ETextureType GetTextureType(int index) const;
	virtual void ResetTexture( int index );

	virtual void ResizeViewport(unsigned int newWidth, unsigned int newHeight);

private:
	void CreateSamplers();
	void CreateConstantBuffers();

private:
	unsigned int m_vpWidth;
	unsigned int m_vpHeight;

	ID3D11Device* m_pD3DDevice;
	ID3D11DeviceContext* m_pD3DDeviceContext;
	IDXGISwapChain*	m_pDXIGSwapChain;
	D3D_FEATURE_LEVEL m_featureLevel;

	ID3D11InputLayout* m_pInputLayout;
	ID3D11RenderTargetView*	m_pRTV;

	ID3D11PixelShader*		m_pPS;
	ID3D11VertexShader*		m_pVS;

	ID3D11Buffer* m_pPSConstantBuffer;
	SConstantBuffer m_PSConstantBufferData;

	// textures
	ID3D11ShaderResourceView* m_pInputTextures[8];

	// samplers
	ID3D11SamplerState* m_pSamplers[6];
};