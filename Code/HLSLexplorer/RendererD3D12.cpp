#include "PCH.h"
#include "RendererD3D12.h"
#include <d3dcompiler.h>
#include "d3dx12.h"

#include "CompilationDX.h"

#include "DDSTextureLoader12.h"
#include "WICTextureLoader12.h"

// TODO: move to project's linker properties
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace
{
	inline void ThrowIfFailed( HRESULT hr )
	{
		if (FAILED( hr ))
		{
			throw std::exception();
		}
	}

	HRESULT CompileShaderFromFile( const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderTarget, ID3DBlob** ppOutBlob )
	{
		HRESULT hr = S_OK;
		DWORD dwFlags = D3DCOMPILE_ENABLE_STRICTNESS;

		ID3DBlob* pErrorBlob = nullptr;

		hr = D3DCompileFromFile( szFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderTarget,
								 dwFlags, 0, ppOutBlob, &pErrorBlob );

		if (FAILED( hr ))
		{
			OutputDebugStringA( reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()) );
			SAFE_RELEASE( pErrorBlob );

			return hr;
		}

		SAFE_RELEASE( pErrorBlob );

		return S_OK;
	}

	ETextureType TranslateTextureTypeD3D12( D3D12_SRV_DIMENSION srvDimensionD3D12 )
	{
		switch (srvDimensionD3D12)
		{
			case D3D12_SRV_DIMENSION_TEXTURE1D:
				return ETextureType::ETexType_1D;
				break;

			case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
				return ETextureType::ETexType_1DArray;
				break;

			case D3D12_SRV_DIMENSION_TEXTURE2D:
				return ETextureType::ETexType_2D;
				break;

			case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
				return ETextureType::ETexType_2DArray;
				break;

			case D3D12_SRV_DIMENSION_TEXTURE3D:
				return ETextureType::ETexType_3D;
				break;

			case D3D12_SRV_DIMENSION_TEXTURECUBE:
				return ETextureType::ETexType_Cube;
				break;

			case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
				return ETextureType::ETexType_CubeArray;
				break;

			default:
				return ETextureType::ETexType_Invalid;
				break;
		}
	}
} // namespace


// todo: remove duplicates from other translation units (d3d11.cpp etc.)
struct SRendererCreateParams
{
	HWND hwnd;
	unsigned int width;
	unsigned int height;
};


CRendererD3D12::CRendererD3D12()
	: m_device(nullptr)
	, m_bDrawFullscreenTriangle(false)
{

}

//-----------------------------------------------------------------------------
ERendererAPI CRendererD3D12::GetRendererAPI() const
{
	return RENDERER_API_D3D12;
}

//-----------------------------------------------------------------------------
bool CRendererD3D12::Initialize( const SRendererCreateParams& createParams )
{
	UINT dxgiFactoryFlags = 0;

#if _DEBUG
	ID3D12DebugPtr debugInterface;
	if (SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) ) ))
	{
		debugInterface->EnableDebugLayer();
	}

	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// Create DXGI factory.
	IDXGIFactory4Ptr factory;
	ThrowIfFailed( CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &factory ) ) );

	// Create hardware D3D12 device.
	IDXGIAdapter1Ptr hardwareAdapter;
	GetHardwareAdapter( factory.Get(), &hardwareAdapter );

	// Check if D3D12-compatible adapter was found.
	if (!hardwareAdapter.Get())
	{
		OutputDebugString( L"CRITICAL: D3D12 adapter was not found!!!\n" );

		return false;
	}

	// Create D3D12 device
	m_device = CreateDevice( hardwareAdapter );

	// Before creating the swapchain, the command queue must be created first
	m_commandQueue = CreateCommandQueue( m_device, D3D12_COMMAND_LIST_TYPE_DIRECT );

	// Create the swap chain
	m_swapChain = CreateSwapChain( createParams.hwnd, factory, m_commandQueue, createParams.width, createParams.height, NUM_FRAMES );
	m_nCurrentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	// To render to the swap chain's back buffers, a render target view (RTV) 
	// needs to be created for each of the swap chain's back buffers. 
	// A descriptor heap can be considered an array of resource views.
	m_descriptorHeapRTV = CreateDescriptorHeap( m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, NUM_FRAMES, D3D12_DESCRIPTOR_HEAP_FLAG_NONE );
	m_nDescriptorSizeRTV = m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );

	// Get size of descriptor for cbv/srv/uav
	m_nDescriptorSizeCBV_SRV_UAV = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	// Describe and create a constant buffer view (CBV) and shader resource view (SRV) desciptor heap.
	// Flags indicate that this descriptor heap can be bound to the pipeline
	// and that desciptors containted in it can be referenced by a root table.
	// * layout: 1 cbv + 8 srv
	m_descriptorHeapCBVandSRVs = CreateDescriptorHeap( m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1 + 8, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );
	
	// Create multiple of null descriptors for SRVs.
	// This allows to avoid of [ EXECUTION ERROR #646: INVALID_DESCRIPTOR_HANDLE]
	// See https://docs.microsoft.com/en-us/windows/win32/direct3d12/descriptors-overview#null-descriptors
	{
		// We start from index[1] - beginning of SRVs
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle( m_descriptorHeapCBVandSRVs->GetCPUDescriptorHandleForHeapStart(), 1, m_nDescriptorSizeCBV_SRV_UAV );

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		for (int i=0; i < 8; i++)
		{
			m_device->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);
			srvHandle.Offset(1, m_nDescriptorSizeCBV_SRV_UAV);
		}
	}

	// Create frame resources (e.g. RTV for each frame/backbuffer)
	UpdateRenderTargetViews( m_device, m_swapChain, m_descriptorHeapRTV );

	for (UINT i = 0; i < NUM_FRAMES; i++)
	{
		m_commandAllocators[i] = CreateCommandAllocator( m_device, D3D12_COMMAND_LIST_TYPE_DIRECT );
	}

	m_commandList = CreateCommandList( m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_nCurrentBackBufferIndex], true );

	m_fence = CreateFence( m_device );
	m_fenceEvent = CreateEventHandle();

	//-----------------------------------------------------------------------------
	// LoadAssets
	//-----------------------------------------------------------------------------

	// Create constant buffer
	{
		ThrowIfFailed( m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_sceneConstantBuffer)) );

		// Describe and create a constant buffer view
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.SizeInBytes = (sizeof(CRenderer::SConstantBuffer) + 255) & ~255; // CB size is required to be 256-byte aligned
		cbvDesc.BufferLocation = m_sceneConstantBuffer->GetGPUVirtualAddress();
		m_device->CreateConstantBufferView(&cbvDesc, m_descriptorHeapCBVandSRVs->GetCPUDescriptorHandleForHeapStart());

		// Map and init constant buffer. We don't unmap this until app closes.
		// Keeping thins mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);	// we don't intend to read from this resource on the CPU
		ThrowIfFailed( m_sceneConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)) );
		memcpy((void*)m_pCbvDataBegin, &m_PSConstantBufferData, sizeof(m_PSConstantBufferData));
	}

	// Create the root signature
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// The highest version. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 ranges[2]; // Perf tip: order from the most frequent to least frequent.
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 12);			// 1 per-frame cbuffer, starting as b12
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 0);			// 8 infrequently changed textures, starting as t0

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

		// Set static samplers
		D3D12_STATIC_SAMPLER_DESC staticSamplers[6] = {};
		{
			// At first, set common stuff for all samplers
			for (int i=0; i < 6; i++)
			{
				staticSamplers[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
				staticSamplers[i].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				staticSamplers[i].MinLOD = 0.0f;
				staticSamplers[i].MaxLOD = D3D12_FLOAT32_MAX;
				staticSamplers[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
				staticSamplers[i].MipLODBias = 0.0f;

				staticSamplers[i].ShaderRegister = i;
				staticSamplers[i].RegisterSpace = 0;
			}
			
			// Point Clamp
			staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

			// Point Wrap
			staticSamplers[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			staticSamplers[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplers[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplers[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

			// Linear Clamp
			staticSamplers[2].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			staticSamplers[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplers[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplers[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

			// Linear Wrap
			staticSamplers[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			staticSamplers[3].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplers[3].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplers[3].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

			// Aniso Clamp
			staticSamplers[4].Filter = D3D12_FILTER_ANISOTROPIC;
			staticSamplers[4].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplers[4].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplers[4].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			staticSamplers[4].MaxAnisotropy = 16;

			// Aniso Wrap
			staticSamplers[5].Filter = D3D12_FILTER_ANISOTROPIC;
			staticSamplers[5].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplers[5].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplers[5].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSamplers[5].MaxAnisotropy = 16;
		}

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1( _countof(rootParameters), rootParameters, 6, staticSamplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT );

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;

		ThrowIfFailed( D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error) );
		ThrowIfFailed( m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)) );
	}

	// At the end, update cbuffer params
	CRenderer::ResizeViewport(createParams.width, createParams.height);

	return true;
}  

//-----------------------------------------------------------------------------
void CRendererD3D12::Cleanup()
{
	Flush();
}

//-----------------------------------------------------------------------------
void CRendererD3D12::Update()
{
	CRenderer::Update();

	memcpy((void*)m_pCbvDataBegin, &m_PSConstantBufferData, sizeof(m_PSConstantBufferData));
}

//-----------------------------------------------------------------------------
void CRendererD3D12::Render()
{
	PopulateCommandList();

	ID3D12CommandList* const commandLists[] = { m_commandList.Get() };

	// IMPORTANT: The call to ID3D12CommandQueue::ExecuteCommandLists does NOT block the calling thread
	// until the commands in the list are finished (!)
	m_commandQueue->ExecuteCommandLists( _countof( commandLists ), commandLists );

	// Present to backbuffer
	// For DXGI_PRESENT flags, see: https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-present

	const UINT syncInterval = 1; // vsync on
	const UINT presentFlags = (syncInterval == 0 && m_bTearingSupport) ? DXGI_PRESENT_ALLOW_TEARING : 0;
	ThrowIfFailed( m_swapChain->Present( syncInterval, presentFlags ) );

	// Immediately after presenting the rendered frame to the screen, a signal is inserted into the queue.
	// This does not block the calling thread but instead returns a value to wait for.
	m_frameFenceValue[m_nCurrentBackBufferIndex] = Signal( m_commandQueue, m_fence, m_fenceValue );

	// After signaling the command queue, the index of the current back buffer is updated.
	m_nCurrentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Block CPU thread until the specified fence value has been reached
	WaitForFenceValue( m_fence, m_frameFenceValue[m_nCurrentBackBufferIndex], m_fenceEvent );
}

//-----------------------------------------------------------------------------
void CRendererD3D12::UpdatePixelShader( const void* dxbcData, unsigned int size, EShaderProfile shaderProfile )
{
	// First of all, make sure that everything can be finished before reseting current pipeline state.
	Flush();

	m_pipelineState.Reset();

	// We need to be consistent with used Shader Model throughout whole PSO.
	// We would like to support both Shader Model 5.0 and 6.0 in real-time pixel shader preview, so we want
	// to have access  to both versions of VS.

	ComPtr<ID3DBlob> blobVS;
	if ( IsShaderProfile6(shaderProfile) )
	{
		// TODO: I'm pretty sure I can do better than... this.
		FILE* f = fopen("FullscreenVS.hlsl", "r");
		if (!f)
		{
			m_bDrawFullscreenTriangle = false;
			return;
		}

		fseek(f, 0, SEEK_END);
		int file_size = ftell(f);
		fseek(f, 0, SEEK_SET);

		char* data = new char[file_size];
		memset(data, 0, sizeof(char) * file_size);
		fread(data, 1, file_size, f);
		fclose(f);
		
		std::vector<unsigned char> vs_60_blob;
		nmCompile::CompileModern_Simple( data, L"QuadVS", L"vs_6_0", vs_60_blob);
		
		delete [] data;

		ThrowIfFailed( D3DCreateBlob(vs_60_blob.size(), &blobVS) );
		memcpy(blobVS->GetBufferPointer(), vs_60_blob.data(), vs_60_blob.size() );		
	}
	else
	{
		// Shader model 4, 5.
		ThrowIfFailed( CompileShaderFromFile( L"FullscreenVS.hlsl", "QuadVS", "vs_5_0", &blobVS ) );
	}

	// Create new Pipeline State Object
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };
	psoDesc.VS = { blobVS->GetBufferPointer(), blobVS->GetBufferSize() };
	psoDesc.PS = { dxbcData, size };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	m_bDrawFullscreenTriangle = true;
}

//-----------------------------------------------------------------------------
void CRendererD3D12::ResetTexture( int index )
{
	// Wait to complete draw calls
	Flush();

	// Reset texture - set null descriptor.
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(m_descriptorHeapCBVandSRVs->GetCPUDescriptorHandleForHeapStart(), 1 + index, m_nDescriptorSizeCBV_SRV_UAV);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_device->CreateShaderResourceView( nullptr, &srvDesc, srvHandle );

	m_textures[index].Reset();
}

//-----------------------------------------------------------------------------
bool CRendererD3D12::LoadTextureFromFile( const wchar_t* path, int index )
{
	// Take care of proper sync. Make sure to finish all currently processed tasks
	// (Apparently this is not needed here!)
	//Flush(m_commandQueue, m_fence, m_fenceValue, m_fenceEvent);


	std::wstring fileName( path );

	std::unique_ptr<uint8_t[]> imageData;
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;

	ID3D12ResourcePtr textureUploadHeap;

	if (!fileName.substr(fileName.length() - 4).compare(std::wstring(L".dds")))
	{
		ThrowIfFailed( DirectX::LoadDDSTextureFromFile(m_device.Get(), path, &m_textures[index], imageData, subresources) );
	}
	else
	{
		subresources.push_back( {} );
		ThrowIfFailed( DirectX::LoadWICTextureFromFile(m_device.Get(), path, &m_textures[index], imageData, subresources[0]) );
	}

	D3D12_RESOURCE_DESC resDesc = m_textures[index]->GetDesc();

	const UINT64 subresourceSize = static_cast<UINT>( subresources.size() );
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_textures[index].Get(), 0, subresourceSize);

	// Create the GPU upload buffer.
	ThrowIfFailed( m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUploadHeap)) );

	// temp commandlist, make sure it's OPEN after creation
	ID3D12GraphicsCommandListPtr tempCommandlist = CreateCommandList( m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_nCurrentBackBufferIndex], false );

	UpdateSubresources( tempCommandlist.Get(), m_textures[index].Get(), textureUploadHeap.Get(), 0, 0, subresourceSize, subresources.data() );

	tempCommandlist->ResourceBarrier(1,
							&CD3DX12_RESOURCE_BARRIER::Transition(
							m_textures[index].Get(), 
							D3D12_RESOURCE_STATE_COPY_DEST,
							D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ) );


	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = resDesc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = subresourceSize;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = -1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle( m_descriptorHeapCBVandSRVs->GetCPUDescriptorHandleForHeapStart() );
	srvHandle.Offset(1, m_nDescriptorSizeCBV_SRV_UAV);	// Need to offset here since the first object in heap in constant buffer
	
	if (index > 0)
	{
		srvHandle.Offset(index, m_nDescriptorSizeCBV_SRV_UAV);
	}

	m_device->CreateShaderResourceView(m_textures[index].Get(), &srvDesc, srvHandle);

	tempCommandlist->DiscardResource( textureUploadHeap.Get(), nullptr );

	ThrowIfFailed( tempCommandlist->Close() );
	ID3D12CommandList* ppCommandLists[] = { tempCommandlist.Get() };
	m_commandQueue->ExecuteCommandLists( _countof( ppCommandLists ), ppCommandLists );

	// Synchronize
	Flush();


	return true;
}

//-----------------------------------------------------------------------------
ETextureType CRendererD3D12::GetTextureType( int index ) const
{
	// TODO: Investigate if there is a way of getting D3D12_SHADER_RESOURCE_DESC somehow.

	// Locate proper SRV
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle;
	srvHandle.Offset(1, m_nDescriptorSizeCBV_SRV_UAV);
	srvHandle.Offset(index, m_nDescriptorSizeCBV_SRV_UAV);

	D3D12_RESOURCE_DESC resDesc = m_textures[index]->GetDesc();

	switch (resDesc.Dimension)
	{
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			return ETextureType::ETexType_2D;

		default:
			return ETextureType::ETexType_Invalid;
	}

	return ETextureType::ETexType_2D;
}

//-----------------------------------------------------------------------------
void CRendererD3D12::ResizeViewport( unsigned int newWidth, unsigned int newHeight )
{
	// Performing resizing swap buffers in the proper way.
	// See https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12Fullscreen/src/D3D12Fullscreen.cpp
	if ( (newWidth != m_vpWidth) || (newHeight != m_vpHeight) )
	{
		CRenderer::ResizeViewport(newWidth, newHeight);

		// Flush all current GPU commands
		Flush();

		// Release the resources holding references to the swap chain (requirement of
		// IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
		// current fence value.
		for (UINT n = 0; n < NUM_FRAMES; n++)
		{
			m_backBuffers[n].Reset();
			m_frameFenceValue[n] = m_frameFenceValue[m_nCurrentBackBufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		ThrowIfFailed( m_swapChain->GetDesc1(&scDesc) );
		ThrowIfFailed( m_swapChain->ResizeBuffers( NUM_FRAMES, newWidth, newHeight, scDesc.Format, scDesc.Flags) );

		m_nCurrentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
		m_vpWidth = newWidth;
		m_vpHeight = newHeight;

		// Here - update all size-dependent resources
		UpdateRenderTargetViews( m_device, m_swapChain, m_descriptorHeapRTV );
	}	
}

Vec4 CRendererD3D12::GetColorAtCursorPosition( unsigned int& x, unsigned int& y ) const
{
	x = m_PSConstantBufferData.cursorPos[0];
	y = m_PSConstantBufferData.cursorPos[1];

	return Vec4();
}

//-----------------------------------------------------------------------------
void CRendererD3D12::GetHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter )
{
	IDXGIAdapter1Ptr adapter;
	*ppAdapter = nullptr;

	UINT destAdapterIndex = 0;
	SIZE_T videoMemory = 0;
	bool adapterFound = false;

	for ( UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); adapterIndex++ )
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		adapter->GetDesc1(&adapterDesc);

		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		// Check if adapter supports D3D12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
		{
			if (adapterDesc.DedicatedVideoMemory > videoMemory)
			{
				videoMemory = adapterDesc.DedicatedVideoMemory;
				destAdapterIndex = adapterIndex;

				// At this point we're sure that we have at least one adapter
				// which supports D3D12
				adapterFound = true;
			}
		}
	}

	if (adapterFound)
	{
		pFactory->EnumAdapters1(destAdapterIndex, ppAdapter);
	}
}

//-----------------------------------------------------------------------------
ID3D12DevicePtr CRendererD3D12::CreateDevice( IDXGIAdapter1Ptr adapter )
{
	ID3D12DevicePtr d3d12Device;
	ThrowIfFailed( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3d12Device) ));

	// TODO: Add some control of debug messages in debug mode

	return d3d12Device;
}

//-----------------------------------------------------------------------------
ID3D12CommandQueuePtr CRendererD3D12::CreateCommandQueue( ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type )
{
	ID3D12CommandQueuePtr d3d12CommandQueue;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed( device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

	return d3d12CommandQueue;
}

//-----------------------------------------------------------------------------
IDXGISwapChain4Ptr CRendererD3D12::CreateSwapChain( HWND hwnd, IDXGIFactory4Ptr factory, ID3D12CommandQueuePtr commandQueue, const UINT width, const UINT height, const UINT bufferCount )
{
	IDXGISwapChain4Ptr dxgiSwapChain4;

	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.SampleDesc = {1, 0};
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = bufferCount;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0; // TODO: Check for tearing

	// Check tearing support
	CheckTearingSupport();
	if (m_bTearingSupport)
		desc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	IDXGISwapChain1Ptr dxgiSwapChain1;
	ThrowIfFailed( factory->CreateSwapChainForHwnd( commandQueue.Get(), hwnd, &desc, nullptr, nullptr, &dxgiSwapChain1) );

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen will be handled manually.
	ThrowIfFailed( factory->MakeWindowAssociation( hwnd, DXGI_MWA_NO_ALT_ENTER ) );

	ThrowIfFailed( dxgiSwapChain1.As( &dxgiSwapChain4 ) );

	m_vpWidth = width;
	m_vpHeight = height;

	return dxgiSwapChain4;
}

//-----------------------------------------------------------------------------
ID3D12DescriptorHeapPtr CRendererD3D12::CreateDescriptorHeap( ID3D12DevicePtr device, D3D12_DESCRIPTOR_HEAP_TYPE type, const UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags )
{
	// A view in DirectX 12 is also called a descriptor.
	// Similar to a view, a descriptor describes a resource.
	// Since the swap chain contains multiple back buffer textures, one descriptor is needed to describe each back buffer texture.
	// The m_RTVDescriptorHeap variable is used to store the descriptor heap that contains the render target views for the swap chain back buffers.

	ID3D12DescriptorHeapPtr descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = flags;
	desc.NodeMask = 0;

	ThrowIfFailed( device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)) );

	return descriptorHeap;
}

//-----------------------------------------------------------------------------
ID3D12CommandAllocatorPtr CRendererD3D12::CreateCommandAllocator( ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type )
{
	ID3D12CommandAllocatorPtr commandAllocator;
	ThrowIfFailed( device->CreateCommandAllocator( type, IID_PPV_ARGS( &commandAllocator ) ) );

	return commandAllocator;
}

//-----------------------------------------------------------------------------
ID3D12GraphicsCommandListPtr CRendererD3D12::CreateCommandList( ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocatorPtr allocator, bool bClose /*= true */ )
{
	ID3D12GraphicsCommandListPtr commandList;
	ThrowIfFailed( device->CreateCommandList( 0, type, allocator.Get(), nullptr, IID_PPV_ARGS( &commandList ) ) );

	if (bClose)
	{
		// By default, the command list is created in recording state.
		// We may want to close it.
		ThrowIfFailed( commandList->Close() );
	}

	return commandList;
}

//-----------------------------------------------------------------------------
ID3D12FencePtr CRendererD3D12::CreateFence( ID3D12DevicePtr device )
{
	ID3D12FencePtr fence;
	ThrowIfFailed( device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &fence ) ) );

	return fence;
}

//-----------------------------------------------------------------------------
HANDLE CRendererD3D12::CreateEventHandle()
{
	HANDLE fenceEvent;
	fenceEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );

	return fenceEvent;
}

//-----------------------------------------------------------------------------
void CRendererD3D12::UpdateRenderTargetViews( ID3D12DevicePtr device, IDXGISwapChain4Ptr swapchain, ID3D12DescriptorHeapPtr descriptorHeap )
{
	// In order to iterate the descriptors in a descriptor heap,
	// a handle to the first descriptor in the heap is retrieved.
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle( descriptorHeap->GetCPUDescriptorHandleForHeapStart() );

	for ( UINT i = 0; i < NUM_FRAMES; i++ )
	{
		ThrowIfFailed( swapchain->GetBuffer( i, IID_PPV_ARGS( &m_backBuffers[i] ) ) );

		// Place the backbuffer RTV in place pointed by rtvHandle (I guess)
		device->CreateRenderTargetView( m_backBuffers[i].Get(), nullptr, rtvHandle );
		rtvHandle.Offset( 1, m_nDescriptorSizeRTV );
	}
}

//-----------------------------------------------------------------------------
void CRendererD3D12::PopulateCommandList()
{
	// pointers to the command allocator and back buffer resource are retrieved according to the current back buffer index.
	ID3D12CommandAllocator* commandAllocator = m_commandAllocators[m_nCurrentBackBufferIndex].Get();
	ID3D12Resource* backBuffer = m_backBuffers[m_nCurrentBackBufferIndex].Get();

	// the command allocator and command list are reset. This prepares the command list for recording the next frame.
	ThrowIfFailed( commandAllocator->Reset() );
	m_commandList->Reset(commandAllocator, m_bDrawFullscreenTriangle ? m_pipelineState.Get() : nullptr);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_descriptorHeapCBVandSRVs.Get() };
	m_commandList->SetDescriptorHeaps( _countof(ppHeaps), ppHeaps );

	// set Root Descriptor Tables
	m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapCBVandSRVs->GetGPUDescriptorHandleForHeapStart());

	CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle( m_descriptorHeapCBVandSRVs->GetGPUDescriptorHandleForHeapStart(), 1, m_nDescriptorSizeCBV_SRV_UAV);
	m_commandList->SetGraphicsRootDescriptorTable(1, srvHandle);

	// Viewport and scissor test
	CD3DX12_VIEWPORT viewport(0.0f, 0.0f, (FLOAT) m_vpWidth, (FLOAT) m_vpHeight, 0.0f, 1.0f);
	CD3DX12_RECT scissorRect(0, 0, (LONG) m_vpWidth, (LONG) m_vpHeight);

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);


	// Before the render target can be cleared, it must be transitioned to the RENDER_TARGET state.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET) );


	// Clear RTV
	static const float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE backBufferRTV( m_descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(), m_nCurrentBackBufferIndex, m_nDescriptorSizeRTV );

	m_commandList->ClearRenderTargetView(backBufferRTV, ClearColor, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &backBufferRTV, FALSE, nullptr);


	// draw command
	m_commandList->IASetVertexBuffers(0, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (m_bDrawFullscreenTriangle)
		m_commandList->DrawInstanced(3, 1, 0, 0);

	// Backbuffer transition
	m_commandList->ResourceBarrier( 1, &CD3DX12_RESOURCE_BARRIER::Transition( backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT ) );

	ThrowIfFailed( m_commandList->Close() );
}

//-----------------------------------------------------------------------------
void CRendererD3D12::CheckTearingSupport()
{
	ComPtr<IDXGIFactory6> factory;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	BOOL allowTearing = FALSE;

	if (SUCCEEDED(hr))
	{
		hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
	}

	m_bTearingSupport = SUCCEEDED(hr) && (allowTearing == TRUE);
}

//-----------------------------------------------------------------------------
uint64_t CRendererD3D12::Signal( ID3D12CommandQueuePtr commandQueue, ID3D12FencePtr fence, uint64_t& fenceValue )
{
	//   ID3D12CommandQueue::Signal method:
	// 
	// - Updates a fence to a specified value from the GPU side. Use ID3D12Fence::Signal to set a fence from the CPU side.
	// - Will append a fence value to the end of the command queue.
	//   The fence object that is used to signal the command queue will have its completed value set
	//   to the value of the signal when processing has reached that point in the command queue.
	//   In other words, the completed value for the fence object will be set to the specified fence value only after all of the commands
	//   that were executed on the command queue prior to the signal have finished executing on the GPU.
	//
	//   The call to this Signal does NOT block the calling thread but instead just returns the value to wait for 
	//   before any (writable) GPU resources that are referenced in the command lists can be reused.
	//
	//   Block the CPU thread until any (writeable; like render targets) resources are finished being used.

	fenceValue++;

	uint64_t fenceValueForSignal = fenceValue;
	ThrowIfFailed( m_commandQueue->Signal(fence.Get(), fenceValue) );

	return fenceValueForSignal;
}

//-----------------------------------------------------------------------------
void CRendererD3D12::WaitForFenceValue( ID3D12FencePtr fence, uint64_t fenceValue, HANDLE fenceEvent )
{
	// This method is used to block CPU thread until the specified fence value has been reached

	const UINT64 currentFenceValue = fence->GetCompletedValue();
	if (currentFenceValue < fenceValue)
	{
		// To wait for a fence to reach a specific value on the CPU, use ID3D12Fence::SetEventOnCompletion
		// followed by a call to WaitForSingleObject.

		// * ID3D12Fence::SetEventOnCompletion
		//   Specifies an event that should be fired when the fence reaches a certain value.
		ThrowIfFailed( fence->SetEventOnCompletion(fenceValue, fenceEvent) );

		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

//-----------------------------------------------------------------------------
void CRendererD3D12::Flush( ID3D12CommandQueuePtr commandQueue, ID3D12FencePtr fence, uint64_t& fenceValue, HANDLE fenceEvent )
{
	// The Signal function returns the fence value to wait for. 
	// The WaitForFenceValue function is used to wait for the fence to be signaled with a specified value.
	// The Flush function will block the calling thread until the fence value has been reached.
	// After this function returns, it is safe to release any resources that were referenced by the GPU.

	uint64_t fenceValueForSignal = Signal( commandQueue, fence, fenceValue );
	WaitForFenceValue( fence, fenceValueForSignal, fenceEvent );
}

//-----------------------------------------------------------------------------
void CRendererD3D12::Flush()
{
	Flush(m_commandQueue, m_fence, m_fenceValue, m_fenceEvent);
}
