#pragma once

// Basic D3D11 component
#include <d3d11.h>

class CDriverD3D11
{
public:
	CDriverD3D11();

	bool Initialize( HWND hWnd, unsigned int Width, unsigned int Height );
	void Cleanup();

	void Update();
	void Render();

	bool LoadTextureFromFile(const WCHAR* path, int index);
	void CreatePixelShader(const void* dxbcData, unsigned int size);
	ETextureType GetTextureType(int index) const;

	void ResizeViewport(unsigned int newWidth, unsigned int newHeight);
	void ResetTexture( int index );

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


	// constant buffer
	struct SConstantBuffer
	{
		SConstantBuffer()
		{
			elapsedTime = 0.0f;
			numFrames = 0;
		}

		float elapsedTime;
		std::uint32_t numFrames;
		float pad[2];

		float viewportX;
		float viewportY;
		float viewportInvX;
		float viewportInvY;
	};

	ID3D11Buffer* m_pPSConstantBuffer;
	SConstantBuffer m_PSConstantBufferData;

	// textures
	ID3D11ShaderResourceView* m_pInputTextures[8];

	// samplers
	ID3D11SamplerState* m_pSamplers[6];
};