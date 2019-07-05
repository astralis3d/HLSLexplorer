#include "PCH.h"
#include "RendererD3D12.h"
#include <d3dcompiler.h>
#include "d3dx12.h"
#include <chrono>

// TODO: move to project's linker properties
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

inline void ThrowIfFailed( HRESULT hr )
{
	if (FAILED( hr ))
	{
		throw std::exception();
	}
}

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
bool CRendererD3D12::Initialize( const SRendererCreateParams& createParams )
{
	UINT dxgiFactoryFlags = 0;

#if _DEBUG
	ComPtr<ID3D12Debug> debugInterface;
	if (SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debugInterface ) ) ))
	{
		debugInterface->EnableDebugLayer();
	}

	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// Create DXGI factory.
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed( CreateDXGIFactory2( dxgiFactoryFlags, IID_PPV_ARGS( &factory ) ) );

	// Create hardware D3D12 device.
	ComPtr<IDXGIAdapter1> hardwareAdapter;
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

	// Describe and create a constant buffer view (CBV) desciptor heap.
	// Flags indicate that this descriptor heap can be bound to the pipeline
	// and that desciptors containted in it can be referenced by a root table.
	m_descriptorHeapCBV = CreateDescriptorHeap( m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE );

	// Create frame resources (e.g. RTV for each frame/backbuffer)
	UpdateRenderTargetViews( m_device, m_swapChain, m_descriptorHeapRTV );

	for (UINT i = 0; i < NUM_FRAMES; i++)
	{
		m_commandAllocators[i] = CreateCommandAllocator( m_device, D3D12_COMMAND_LIST_TYPE_DIRECT );
	}

	m_commandList = CreateCommandList( m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_nCurrentBackBufferIndex], true );

	m_fence = CreateFence( m_device );
	m_fenceEvent = CreateEventHandle();

	return true;
}

void CRendererD3D12::Cleanup()
{
	Flush(m_commandQueue, m_fence, m_fenceValue, m_fenceEvent);
}

//-----------------------------------------------------------------------------
void CRendererD3D12::Update()
{
	// TODO
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
	const UINT syncInterval = 1; // vsync on
	const UINT presentFlags = 0;
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
void CRendererD3D12::UpdatePixelShader( const void* dxbcData, unsigned int size )
{
	m_pipelineState.Reset();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { };

	psoDesc.PS = { dxbcData, size };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC( D3D12_DEFAULT );
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	m_bDrawFullscreenTriangle = true;
}

void CRendererD3D12::ResetTexture( int index )
{

}

bool CRendererD3D12::LoadTextureFromFile( const wchar_t* path, int index )
{
	return false; // todo
}

ETextureType CRendererD3D12::GetTextureType( int index ) const
{
	return ETextureType::ETexType_Invalid; // todo
}

void CRendererD3D12::ResizeViewport( unsigned int newWidth, unsigned int newHeight )
{

}

//-----------------------------------------------------------------------------
void CRendererD3D12::GetHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter )
{
	ComPtr<IDXGIAdapter1> adapter;
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
ComPtr<ID3D12Device> CRendererD3D12::CreateDevice( ComPtr<IDXGIAdapter1> adapter )
{
	ComPtr<ID3D12Device> d3d12Device;
	ThrowIfFailed( D3D12CreateDevice( adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&d3d12Device) ));

	// TODO: Add some control of debug messages in debug mode

	return d3d12Device;
}

//-----------------------------------------------------------------------------
ComPtr<ID3D12CommandQueue> CRendererD3D12::CreateCommandQueue( ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type )
{
	ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed( device->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue)));

	return d3d12CommandQueue;
}

//-----------------------------------------------------------------------------
ComPtr<IDXGISwapChain4> CRendererD3D12::CreateSwapChain( HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> commandQueue, const UINT width, const UINT height, const UINT bufferCount )
{
	ComPtr<IDXGISwapChain4> dxgiSwapChain4;

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

	ComPtr<IDXGISwapChain1> dxgiSwapChain1;
	ThrowIfFailed( factory->CreateSwapChainForHwnd( commandQueue.Get(), hwnd, &desc, nullptr, nullptr, &dxgiSwapChain1) );

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen will be handled manually.
	ThrowIfFailed( factory->MakeWindowAssociation( hwnd, DXGI_MWA_NO_ALT_ENTER ) );

	ThrowIfFailed( dxgiSwapChain1.As( &dxgiSwapChain4 ) );

	return dxgiSwapChain4;
}

//-----------------------------------------------------------------------------
ComPtr<ID3D12DescriptorHeap> CRendererD3D12::CreateDescriptorHeap( ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, const UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags )
{
	// A view in DirectX 12 is also called a descriptor.
	// Similar to a view, a descriptor describes a resource.
	// Since the swap chain contains multiple back buffer textures, one descriptor is needed to describe each back buffer texture.
	// The m_RTVDescriptorHeap variable is used to store the descriptor heap that contains the render target views for the swap chain back buffers.

	ComPtr<ID3D12DescriptorHeap> descriptorHeap;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;
	desc.Flags = flags;
	desc.NodeMask = 0;

	ThrowIfFailed( device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)) );

	return descriptorHeap;
}

//-----------------------------------------------------------------------------
ComPtr<ID3D12CommandAllocator> CRendererD3D12::CreateCommandAllocator( ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type )
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ThrowIfFailed( device->CreateCommandAllocator( type, IID_PPV_ARGS( &commandAllocator ) ) );

	return commandAllocator;
}

//-----------------------------------------------------------------------------
ComPtr<ID3D12GraphicsCommandList> CRendererD3D12::CreateCommandList( ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> allocator, bool bClose /*= true */ )
{
	ComPtr<ID3D12GraphicsCommandList> commandList;
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
ComPtr<ID3D12Fence> CRendererD3D12::CreateFence( ComPtr<ID3D12Device> device )
{
	ComPtr<ID3D12Fence> fence;
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
void CRendererD3D12::UpdateRenderTargetViews( ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swapchain, ComPtr<ID3D12DescriptorHeap> descriptorHeap )
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
	m_commandList->Reset(commandAllocator, m_bDrawFullscreenTriangle ? m_pipelineState.Get() : nullptr); // todo: set default PSO

	// Viewport and scissor test
	CD3DX12_VIEWPORT viewport(0.0f, 0.0f, (FLOAT) m_vpWidth, (FLOAT) m_vpHeight, 0.0f, 1.0f);
	CD3DX12_RECT scissorRect(0, 0, (LONG) m_vpWidth, (LONG) m_vpHeight);

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	// Before the render target can be cleared, it must be transitioned to the RENDER_TARGET state.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET) );

	static const float ClearColor[4] = { 0.0f, 0.0f, 0.8f, 0.0f };

	// Clear RTV
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
uint64_t CRendererD3D12::Signal( ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue )
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
void CRendererD3D12::WaitForFenceValue( ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent )
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
void CRendererD3D12::Flush( ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent )
{
	// The Signal function returns the fence value to wait for. 
	// The WaitForFenceValue function is used to wait for the fence to be signaled with a specified value.
	// The Flush function will block the calling thread until the fence value has been reached.
	// After this function returns, it is safe to release any resources that were referenced by the GPU.

	uint64_t fenceValueForSignal = Signal( commandQueue, fence, fenceValue );
	WaitForFenceValue( fence, fenceValueForSignal, fenceEvent );
}
