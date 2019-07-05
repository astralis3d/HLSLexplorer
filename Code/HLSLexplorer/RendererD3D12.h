#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "Renderer.h"

struct SRendererCreateParams;

using Microsoft::WRL::ComPtr;

class CRendererD3D12 : public CRenderer
{
public:
	CRendererD3D12();
	virtual bool Initialize(const SRendererCreateParams& createParams);
	virtual void Cleanup();

	virtual void Update();
	virtual void Render();

	virtual void UpdatePixelShader(const void* dxbcData, unsigned int size);
	virtual void ResetTexture( int index );
	virtual bool LoadTextureFromFile(const wchar_t* path, int index);
	virtual ETextureType GetTextureType(int index) const;

	virtual void ResizeViewport(unsigned int newWidth, unsigned int newHeight);

private:
	void GetHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter );
	ComPtr<ID3D12Device> CreateDevice( ComPtr<IDXGIAdapter1> adapter );
	ComPtr<ID3D12CommandQueue> CreateCommandQueue( ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type );
	ComPtr<IDXGISwapChain4> CreateSwapChain( HWND hwnd, ComPtr<IDXGIFactory4> factory, ComPtr<ID3D12CommandQueue> commandQueue, const UINT width, const UINT height, const UINT bufferCount );
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap( ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, const UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags );
	ComPtr<ID3D12CommandAllocator> CreateCommandAllocator( ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type );
	ComPtr<ID3D12GraphicsCommandList> CreateCommandList( ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12CommandAllocator> allocator, bool bClose = true );
	void UpdateRenderTargetViews( ComPtr<ID3D12Device> device, ComPtr<IDXGISwapChain4> swapchain, ComPtr<ID3D12DescriptorHeap> descriptorHeap );

	ComPtr<ID3D12Fence> CreateFence( ComPtr<ID3D12Device> device );
	HANDLE CreateEventHandle();

	void PopulateCommandList();

	uint64_t Signal( ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue );
	void WaitForFenceValue( ComPtr<ID3D12Fence> fence, uint64_t fenceValue, HANDLE fenceEvent );
	void Flush( ComPtr<ID3D12CommandQueue> commandQueue, ComPtr<ID3D12Fence> fence, uint64_t& fenceValue, HANDLE fenceEvent );

	static const UINT NUM_FRAMES = 2;

private:
	ComPtr<ID3D12Device>		m_device;

	ComPtr<ID3D12CommandQueue>	m_commandQueue;
	ComPtr<IDXGISwapChain4>		m_swapChain;
	ComPtr<ID3D12Resource>		m_backBuffers[NUM_FRAMES];
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[NUM_FRAMES];

	ComPtr<ID3D12DescriptorHeap> m_descriptorHeapRTV;
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeapCBV;

	ComPtr<ID3D12PipelineState>	m_pipelineState;

	UINT m_nCurrentBackBufferIndex;

	UINT m_nDescriptorSizeRTV;
	//UINT m_nDescriptorSizeCBV;

	ComPtr<ID3D12Fence> m_fence;
	uint64_t m_fenceValue = 0;
	uint64_t m_frameFenceValue[NUM_FRAMES];
	HANDLE m_fenceEvent;

	unsigned int m_vpWidth;
	unsigned int m_vpHeight;

	bool m_bDrawFullscreenTriangle;
};