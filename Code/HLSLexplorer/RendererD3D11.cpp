#include "PCH.h"
#include "RendererD3D11.h"
#include <d3dcompiler.h>
#include <VersionHelpers.h>
#include <algorithm>
#include "DummyShaders.h"

#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#include "ScreenGrab/ScreenGrab.h"
#include <wincodec.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct SRendererCreateParams
{
	HWND hwnd;
	unsigned int width;
	unsigned int height;
};


// Compiles shader
HRESULT CompileShader( const void* sourceData, UINT sourceLength, LPCSTR szEntryPoint, LPCSTR szShaderTarget, ID3DBlob** ppOutBlob )
{
	HRESULT hr = S_OK;
	DWORD dwFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompile(sourceData, sourceLength, nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderTarget, dwFlags, 0, ppOutBlob, &pErrorBlob);

	if ( FAILED(hr) && pErrorBlob)
	{
		OutputDebugStringA( reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()) );	
	}		

	SAFE_RELEASE( pErrorBlob );

	return hr;
}

namespace
{
	ETextureType TranslateTextureTypeD3D11(D3D11_SRV_DIMENSION srvDimensionD3D11)
	{
		switch (srvDimensionD3D11)
		{
			case D3D11_SRV_DIMENSION_TEXTURE1D:	
				return ETextureType::ETexType_1D;
				break;

			case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:
				return ETextureType::ETexType_1DArray;
				break;

			case D3D11_SRV_DIMENSION_TEXTURE2D:
				return ETextureType::ETexType_2D;
				break;

			case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
				return ETextureType::ETexType_2DArray;
				break;

			case D3D11_SRV_DIMENSION_TEXTURE3D:	
				return ETextureType::ETexType_3D;
				break;

			case D3D11_SRV_DIMENSION_TEXTURECUBE:
				return ETextureType::ETexType_Cube;
				break;

			case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
				return ETextureType::ETexType_CubeArray;
				break;

			default:
				return ETextureType::ETexType_Invalid;
				break;
		}
	}
} // namespace


CRendererD3D11::CRendererD3D11() 
	: m_pD3DDevice(nullptr)
	, m_pD3DDeviceContext(nullptr)
	, m_SwapChain(nullptr)
	, m_pVS( nullptr )
	, m_pPS( nullptr )
{
	for (int i=0; i < 8; i++)
	{
		m_pInputTextures[i] = nullptr;
	}

	for (int i=0; i < 6; i++)
	{
		m_pSamplers[i] = nullptr;
	}
}

//-----------------------------------------------------------------------------
ERendererAPI CRendererD3D11::GetRendererAPI() const
{
	return RENDERER_API_D3D11;
}

bool CRendererD3D11::LoadTextureFromFile( const WCHAR* path, int index )
{
	const std::wstring fileName(path);
	HRESULT hr = E_FAIL;

	SAFE_RELEASE( m_pInputTextures[index] );
	if (!fileName.substr(fileName.length() - 4).compare(L".dds"))
	{
		hr = DirectX::CreateDDSTextureFromFile( m_pD3DDevice, path, nullptr, &m_pInputTextures[index] );
	}
	else
	{
		hr = DirectX::CreateWICTextureFromFile( m_pD3DDevice, path, nullptr, &m_pInputTextures[index] );
	}

	const bool bSuccess = (hr == S_OK);
	if (bSuccess)
	{
		m_pD3DDeviceContext->PSSetShaderResources( index, 1, &m_pInputTextures[index] );
	}

	return bSuccess;
}

ETextureType CRendererD3D11::GetTextureType( int index ) const
{
	if (!m_pInputTextures[index])
	{
		return ETextureType::ETexType_Invalid;
	}
	else
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		m_pInputTextures[index]->GetDesc( &srvDesc );

		return TranslateTextureTypeD3D11(srvDesc.ViewDimension);
	}
}

bool CRendererD3D11::Initialize(const SRendererCreateParams& createParams)
{
	m_vpWidth = createParams.width;
	m_vpHeight = createParams.height;


	// Init D3D11 device
	HRESULT hr = E_FAIL;

	UINT createD3D11DeviceFlags = 0;
#ifdef _DEBUG
	createD3D11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] = 
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numDriverTypes = ARRAYSIZE( driverTypes );


	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	for ( UINT iDriverType = 0; iDriverType < numDriverTypes; iDriverType++ )
	{
		const D3D_DRIVER_TYPE selectedDriverType = driverTypes[ iDriverType ];

		hr = D3D11CreateDevice(nullptr, selectedDriverType, nullptr, createD3D11DeviceFlags, featureLevels, numFeatureLevels,
								D3D11_SDK_VERSION, &m_pD3DDevice, &m_featureLevel, &m_pD3DDeviceContext);

		if ( hr == E_INVALIDARG )
			break;

		if ( SUCCEEDED(hr) )
			break;
	}

	if ( FAILED(hr) )
		return false;

	// Obtain DXGI factory from the device
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = m_pD3DDevice->QueryInterface( __uuidof(IDXGIDevice), (void**) &dxgiDevice );
		if ( SUCCEEDED(hr) )
		{
			IDXGIAdapter* pAdapter = nullptr;
			hr  = dxgiDevice->GetAdapter(&pAdapter);
			if ( SUCCEEDED(hr) )
			{
				hr = pAdapter->GetParent( __uuidof(IDXGIFactory1), (void**) &dxgiFactory);
				pAdapter->Release();
			}
			dxgiDevice->Release();
		}
	}

	if ( FAILED(hr) )
		return false;

	// Create swap chain
	{
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width =  m_vpWidth;
		sd.BufferDesc.Height = m_vpHeight;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SwapEffect = IsWindows8OrGreater() ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD;
		sd.OutputWindow = createParams.hwnd;
		sd.Windowed = TRUE;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;

		hr = dxgiFactory->CreateSwapChain( m_pD3DDevice, &sd, &m_SwapChain );
		if (FAILED(hr))
		{
			return false;
		}
	}

	// block alt+enter
	dxgiFactory->MakeWindowAssociation( createParams.hwnd, DXGI_MWA_NO_ALT_ENTER );
	dxgiFactory->Release();
	
	// Create render target view
	{
		ID3D11Texture2D* pBackBuffer = nullptr;

		hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBackBuffer );
		if ( FAILED(hr) )
		{
			return false;
		}

		hr = m_pD3DDevice->CreateRenderTargetView( pBackBuffer, nullptr, &m_pRTV );
		pBackBuffer->Release();

		if (FAILED( hr ))
		{
			return false;
		}
	}

	// Compile Vertex Shader
	{
		ID3DBlob* pBlobVS = nullptr;

		hr = CompileShader( szFullscreenVS, ARRAYSIZE(szFullscreenVS), "QuadVS", "vs_5_0", &pBlobVS );
		if ( FAILED(hr) )
		{
			::MessageBox(nullptr, L"Unable to compile vertex shader", L"Error", MB_OK | MB_ICONERROR );
			return hr;
		}

		hr = m_pD3DDevice->CreateVertexShader( (const void*) pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), nullptr, &m_pVS );
		if ( FAILED(hr) )
		{
			::MessageBox( nullptr, L"Unable to create vertex shader", L"Error", MB_OK | MB_ICONERROR );
			SAFE_RELEASE( m_pVS );

			return hr;
		}
	}

	CreateSamplers();
	CreateConstantBuffers();

	// At the end, update cbuffer params
	CRenderer::ResizeViewport( createParams.width, createParams.height );

	{
		D3D11_TEXTURE2D_DESC texDesc = {};

		texDesc.Width = 1;
		texDesc.Height = 1;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		texDesc.Usage = D3D11_USAGE_STAGING;
		texDesc.SampleDesc = {1, 0};
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.ArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.BindFlags = 0;

		HRESULT hr = m_pD3DDevice->CreateTexture2D(&texDesc, nullptr, &m_copyTex2D);
		if (FAILED(hr))
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// simple rendering
void CRendererD3D11::Render()
{
	ID3D11DeviceContext* pDevCon = m_pD3DDeviceContext;

	float ClearColor[4] = { 0.f, 0.f, 0.f, 0.f };
	pDevCon->ClearRenderTargetView( m_pRTV, ClearColor );
	pDevCon->OMSetRenderTargets( 1, &m_pRTV, nullptr );

	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = m_vpWidth;
	vp.Height = m_vpHeight;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pDevCon->RSSetViewports(1, &vp);

	pDevCon->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	pDevCon->IASetInputLayout( nullptr );
	pDevCon->IASetVertexBuffers( 0, 0, nullptr, nullptr, nullptr );

	pDevCon->VSSetShader(m_pVS, nullptr, 0);
	pDevCon->PSSetShader(m_pPS, nullptr, 0);

	pDevCon->PSSetSamplers(0, 6, m_pSamplers );

	// Update and set constant buffer for Pixel Shader
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		m_pD3DDeviceContext->Map( m_pPSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );

		SConstantBuffer* pBuffer = reinterpret_cast<SConstantBuffer*>( mappedResource.pData );
		*pBuffer = m_PSConstantBufferData;

		m_pD3DDeviceContext->Unmap( m_pPSConstantBuffer, 0 );
	}

	pDevCon->PSSetConstantBuffers(12, 1, &m_pPSConstantBuffer);

	pDevCon->Draw( 3, 0 );



	ID3D11Texture2D* pBackBuffer = nullptr;
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBackBuffer );

	D3D11_TEXTURE2D_DESC tex2DDesc = {};
	pBackBuffer->GetDesc(&tex2DDesc);

	// Avoid getting value of pixel out of bounds. This can happen when having a fullscreen and then reducing
	// the size of window.
	const unsigned int cursor_x = std::min<unsigned int>(m_PSConstantBufferData.cursorPos[0], tex2DDesc.Width - 1);
	const unsigned int cursor_y = std::min<unsigned int>(m_PSConstantBufferData.cursorPos[1], tex2DDesc.Height - 1);

	D3D11_BOX box;
	box.left = cursor_x;
	box.right = cursor_x + 1;
	box.top = cursor_y;
	box.bottom = cursor_y + 1;
	box.front = 0;
	box.back = 1;

	pDevCon->CopySubresourceRegion(m_copyTex2D, 0, 0, 0, 0, pBackBuffer, 0, &box);

	pBackBuffer->Release();

	// map the staging buffer with content of selected pixel
	D3D11_MAPPED_SUBRESOURCE mappedSubr;
	pDevCon->Map(m_copyTex2D, 0, D3D11_MAP_READ, 0, &mappedSubr);
	m_colorData = *reinterpret_cast<Vec4*>(mappedSubr.pData);
	pDevCon->Unmap(m_copyTex2D, 0);


	// present back buffer to screen
	m_SwapChain->Present(1, 0);
}

//-----------------------------------------------------------------------------
void CRendererD3D11::UpdatePixelShader( const void* dxbcData, unsigned int size, EShaderProfile shaderProfile )
{
	SAFE_RELEASE( m_pPS );

	HRESULT hr = m_pD3DDevice->CreatePixelShader( dxbcData, size, nullptr, &m_pPS );

	if (FAILED( hr ))
	{
		// output debug info or sth
	}
}

//-----------------------------------------------------------------------------
void CRendererD3D11::Cleanup()
{
	SAFE_RELEASE( m_pD3DDevice );
	SAFE_RELEASE( m_pD3DDeviceContext );
	SAFE_RELEASE( m_pInputLayout );
	SAFE_RELEASE( m_SwapChain );

	SAFE_RELEASE( m_pVS );
	SAFE_RELEASE( m_pPS );
	SAFE_RELEASE( m_pRTV );

	SAFE_RELEASE( m_copyTex2D );

	SAFE_RELEASE( m_pPSConstantBuffer );

	for (int i = 0; i < 8; i++)
	{
		SAFE_RELEASE( m_pInputTextures[i] );
	}

	for (int i = 0; i < 6; i++)
	{
		SAFE_RELEASE (m_pSamplers[i] );
	}
}

//-----------------------------------------------------------------------------
void CRendererD3D11::Update()
{
	CRenderer::Update();
}

//-----------------------------------------------------------------------------
void CRendererD3D11::ResizeViewport( unsigned int newWidth, unsigned int newHeight )
{
	CRenderer::ResizeViewport(newWidth, newHeight);

	m_vpWidth = newWidth;
	m_vpHeight = newHeight;

	if (!m_SwapChain || !m_pD3DDevice || !m_pD3DDevice)
		return;

	SAFE_RELEASE( m_pRTV );

	HRESULT hr = S_OK;
	hr = m_SwapChain->ResizeBuffers( 2, newWidth, newHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0 );

	// Create render target view
	{
		ID3D11Texture2D* pBackBuffer = nullptr;

		hr = m_SwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer );
		if (FAILED(hr) )
		{
			return;
		}

		hr = m_pD3DDevice->CreateRenderTargetView( pBackBuffer, nullptr, &m_pRTV );
		pBackBuffer->Release();

		if (FAILED(hr) )
		{
			return;
		}
	}
}

Vec4 CRendererD3D11::GetColorAtCursorPosition( unsigned int& x, unsigned int& y ) const
{
	x = m_PSConstantBufferData.cursorPos[0];
	y = m_PSConstantBufferData.cursorPos[1];

	return m_colorData;
}

bool CRendererD3D11::SaveTextureToFile( const std::wstring& path )
{
	HRESULT hr = E_FAIL;

	// Get backbuffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &pBackBuffer );
	if ( FAILED(hr) )
	{
		return false;
	}

	// Mark result value as fail
	hr = E_FAIL;

	const std::wstring extension = path.substr( path.length() - 4);
	if (extension == L".dds")
	{
		hr = DirectX::SaveDDSTextureToFile( m_pD3DDeviceContext, pBackBuffer, path.c_str() );
	}
	else if (extension == L".png")
	{
		hr = DirectX::SaveWICTextureToFile( m_pD3DDeviceContext, pBackBuffer, GUID_ContainerFormatPng, path.c_str() );
	}
	else if (extension == L".jpg")
	{
		hr = DirectX::SaveWICTextureToFile( m_pD3DDeviceContext, pBackBuffer, GUID_ContainerFormatJpeg, path.c_str() );
	}
	else if (extension == L".bmp")
	{
		hr = DirectX::SaveWICTextureToFile( m_pD3DDeviceContext, pBackBuffer, GUID_ContainerFormatBmp, path.c_str() );
	}

	// Release refcount for backbuffer
	pBackBuffer->Release();

	return (hr == S_OK);
}

void CRendererD3D11::ResetTexture( int index )
{
	SAFE_RELEASE( m_pInputTextures[index] );

	ID3D11ShaderResourceView* pSRV[1] = { nullptr };
	m_pD3DDeviceContext->PSSetShaderResources(index, 1, pSRV);
}

void CRendererD3D11::CreateConstantBuffers()
{
	D3D11_BUFFER_DESC bufDesc;
	bufDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufDesc.MiscFlags = 0;
	bufDesc.ByteWidth = sizeof(SConstantBuffer);

	HRESULT hr = m_pD3DDevice->CreateBuffer(&bufDesc, nullptr, &m_pPSConstantBuffer);
	if (FAILED( hr ))
	{
		return;
	}
}

void CRendererD3D11::CreateSamplers()
{
	HRESULT hr;

	// Point Clamp
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	memset( &samplerDesc.BorderColor, 0, sizeof( samplerDesc.BorderColor ) );
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	hr = m_pD3DDevice->CreateSamplerState( &samplerDesc, &m_pSamplers[0] );
	if (FAILED(hr))
	{
		return;
	}

	// Point Wrap
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_pD3DDevice->CreateSamplerState( &samplerDesc, &m_pSamplers[1] );
	if (FAILED( hr ))
	{
		return;
	}

	// Linear Clamp
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	hr = m_pD3DDevice->CreateSamplerState( &samplerDesc, &m_pSamplers[2] );
	if (FAILED( hr ))
	{
		return;
	}

	// Linear Wrap
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_pD3DDevice->CreateSamplerState( &samplerDesc, &m_pSamplers[3] );
	if (FAILED( hr ))
	{
		return;
	}

	// Aniso Clamp
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MipLODBias = -1.0f;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = m_pD3DDevice->CreateSamplerState( &samplerDesc, &m_pSamplers[4] );
	if (FAILED( hr ))
	{
		return;
	}

	// Aniso Wrap
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_pD3DDevice->CreateSamplerState( &samplerDesc, &m_pSamplers[5] );
	if (FAILED( hr ))
	{
		return;
	}
}
