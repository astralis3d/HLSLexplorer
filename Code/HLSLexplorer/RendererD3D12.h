#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "Renderer.h"

using Microsoft::WRL::ComPtr;

#define D3D12_COMPTR(x)	typedef ComPtr<x> x##Ptr;

D3D12_COMPTR(IDXGISwapChain1)
D3D12_COMPTR(IDXGISwapChain4)
D3D12_COMPTR(IDXGIAdapter1)
D3D12_COMPTR(IDXGIFactory4)

D3D12_COMPTR(ID3D12Device)
D3D12_COMPTR(ID3D12CommandQueue)
D3D12_COMPTR(ID3D12Resource)
D3D12_COMPTR(ID3D12DescriptorHeap)
D3D12_COMPTR(ID3D12Fence)
D3D12_COMPTR(ID3D12PipelineState)
D3D12_COMPTR(ID3D12RootSignature)
D3D12_COMPTR(ID3D12GraphicsCommandList)
D3D12_COMPTR(ID3D12CommandAllocator)

D3D12_COMPTR(ID3D12Debug)
D3D12_COMPTR(ID3DBlob)

struct SRendererCreateParams;

class CRendererD3D12 : public CRenderer
{
public:
	CRendererD3D12();
	virtual bool Initialize(const SRendererCreateParams& createParams);
	virtual void Cleanup();

	virtual void Update();
	virtual void Render();

	virtual ERendererAPI GetRendererAPI() const;

	virtual void UpdatePixelShader(const void* dxbcData, unsigned int size, EShaderProfile shaderProfile);
	virtual void ResetTexture( int index );
	virtual bool LoadTextureFromFile(const wchar_t* path, int index);
	virtual ETextureType GetTextureType(int index) const;

	virtual void ResizeViewport(unsigned int newWidth, unsigned int newHeight);

	virtual Vec4 GetColorAtCursorPosition(unsigned int& x, unsigned int& y) const;

	virtual bool SaveTextureToFile(const std::wstring& path);

private:
	static const UINT NUM_FRAMES = 2;

	void GetHardwareAdapter( IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter );
	ID3D12DevicePtr CreateDevice( IDXGIAdapter1Ptr adapter );
	ID3D12CommandQueuePtr CreateCommandQueue( ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type );
	IDXGISwapChain4Ptr CreateSwapChain( HWND hwnd, IDXGIFactory4Ptr factory, ID3D12CommandQueuePtr commandQueue, const UINT width, const UINT height, const UINT bufferCount );
	ID3D12DescriptorHeapPtr CreateDescriptorHeap( ID3D12DevicePtr device, D3D12_DESCRIPTOR_HEAP_TYPE type, const UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags );
	ID3D12CommandAllocatorPtr CreateCommandAllocator( ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type );
	ID3D12GraphicsCommandListPtr CreateCommandList( ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocatorPtr allocator, bool bClose = true );
	void UpdateRenderTargetViews( ID3D12DevicePtr device, IDXGISwapChain4Ptr swapchain, ID3D12DescriptorHeapPtr descriptorHeap );

	ID3D12FencePtr CreateFence( ID3D12DevicePtr device );
	HANDLE CreateEventHandle();

	void PopulateCommandList();
	void CreateSamplers();

	void CheckTearingSupport();

	uint64_t Signal( ID3D12CommandQueuePtr commandQueue, ID3D12FencePtr fence, uint64_t& fenceValue );
	void WaitForFenceValue( ID3D12FencePtr fence, uint64_t fenceValue, HANDLE fenceEvent );
	void Flush( ID3D12CommandQueuePtr commandQueue, ID3D12FencePtr fence, uint64_t& fenceValue, HANDLE fenceEvent );
	void Flush();

private:
	ID3D12DevicePtr			m_device;

	ID3D12CommandQueuePtr	m_commandQueue;
	IDXGISwapChain4Ptr		m_swapChain;
	ID3D12ResourcePtr		m_backBuffers[NUM_FRAMES];
	ID3D12GraphicsCommandListPtr m_commandList;
	ID3D12CommandAllocatorPtr m_commandAllocators[NUM_FRAMES];

	ID3D12DescriptorHeapPtr m_descriptorHeapRTV;
	ID3D12DescriptorHeapPtr m_descriptorHeapCBVandSRVs;

	ID3D12ResourcePtr		m_sceneConstantBuffer;
	unsigned char*				m_pCbvDataBegin;

	ID3D12ResourcePtr		m_textures[8];

	ID3D12RootSignaturePtr m_rootSignature;
	ID3D12PipelineStatePtr	m_pipelineState;

	UINT m_nCurrentBackBufferIndex;

	UINT m_nDescriptorSizeCBV_SRV_UAV;
	UINT m_nDescriptorSizeRTV;

	ID3D12FencePtr m_fence;
	uint64_t m_fenceValue = 0;
	uint64_t m_frameFenceValue[NUM_FRAMES];
	HANDLE m_fenceEvent;

	unsigned int m_vpWidth;
	unsigned int m_vpHeight;

	bool	m_bTearingSupport;

	bool m_bDrawFullscreenTriangle;
};