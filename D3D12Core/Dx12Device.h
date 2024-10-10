#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "D3D12Common.h"
#include <vector>
#include "Buffer.h"
#include "Texture.h"
#include "DDSTextureLoader.h"

//#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")


class __declspec(dllexport) Dx12Device
{

public:

	// Public interfaces

	static Dx12Device* GetDevice();

	static void Create();
	static void InitSwapchain(HWND window, int swapchainWidth, int swapchainHeight, bool windowed = true);
	static void DestroySwapchain();
	static void FlushQueue();
	static ID3D12Resource* GetCurrentBackBuffer();
	static Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC* desc);
	static Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc);

	static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	static Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCurrentFrameAllocator();
	static Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue();

	static void Present();

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackBufferView();
	static D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView();

	inline static const DXGI_FORMAT GetBackBufferFormat() { return mBackBufferFormat; }
	inline static const DXGI_FORMAT GetDepthStencilFormat() { return mDepthStencilFormat; }
	inline static const int GetSwapchainBufferCount() { return mSwapChainBufferCount; }

	static std::unique_ptr<Buffer> CreateBuffer(const void* data, UINT64 bufferSize);
	static std::unique_ptr<FrameBuffer> CreateFrameBuffer(const void* data, UINT64 bufferSize);

	static HRESULT LoadTextureFromDDSFile(Texture* texture, UINT mostDetailedMip = 0, FLOAT minLodClamp = 0.0f);

	static ResourceViewHandle GetNextRtvDescriptorHandle();
	static ResourceViewHandle CreateRenderTargetView(ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* viewDesc);

	static ResourceViewHandle GetNextDsvDescriptorHandle();
	static ResourceViewHandle CreateDepthStencilView(ID3D12Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC* viewDesc);

	static ResourceViewHandle GetNextCbvSrvUavDescriptorHandle();
	static ResourceViewHandle CreateConstantBufferView(D3D12_CONSTANT_BUFFER_VIEW_DESC* viewDesc);
	static ResourceViewHandle CreateShaderResourceView(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* viewDesc);
	static ResourceViewHandle CreateUnorderedAccessView(ID3D12Resource* resource, ID3D12Resource* counterResource, D3D12_UNORDERED_ACCESS_VIEW_DESC* viewDesc);

	static const std::array<ID3D12DescriptorHeap*, 3> GetDescriptorHeaps();

private:
	Dx12Device();
	void ResizeSwapchain(int swapchainWidth, int swapchainHeight);
	void FlushCommandQueue();
	void PresentSwapchain();

	static const int mSwapChainBufferCount = 3;

	Microsoft::WRL::ComPtr<IDXGIFactory4> mDxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> mD3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

	/// TODO: Change this
	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc[mSwapChainBufferCount];
	std::array< Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, mSwapChainBufferCount> mDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	
	int mCurrentBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[mSwapChainBufferCount];

	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	UINT mRtvHeapOffset = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	UINT mDsvHeapOffset = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvSrvUavHeap;
	UINT mCbvSrvUavHeapOffset = 0;

	static const DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	UINT mRtvDescriptorSize;
	UINT mDsvDescriptorSize;
	UINT mCbvSrvUavDescriptorSize;

	UINT64 mCurrentFenceVal = 0;

};

