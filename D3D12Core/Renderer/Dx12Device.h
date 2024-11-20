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
#include "GBuffer.h"

//#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include <mutex>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class __declspec(dllexport) Dx12Device
{

public:

	// Public interfaces

	static Dx12Device* GetDevice();

	static void Create();
	static void Destroy();
	static void InitSwapchain(HWND window, int swapchainWidth, int swapchainHeight, bool windowed = true);
	static void DestroySwapchain();
	static void FlushQueue(bool hardFlush = false);
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

	inline static const UINT FrameNumToBufferNum(UINT frameNum) { return frameNum % mSwapChainBufferCount; }

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

	static const std::array<ID3D12DescriptorHeap*, 4> GetDescriptorHeaps();

	static GBuffer* GetGBuffer();

	static D3D12_VIEWPORT GetViewport();

	static void Resize(int width, int height);

	static void InitImGui();

	static bool IsImGuiInited();

	static inline LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	static std::recursive_mutex* GetImGuiIoMutex();


private:
	Dx12Device();
	void ResizeSwapchain(int swapchainWidth, int swapchainHeight);
	void FlushCommandQueue();
	void HardFlushCommandQueue();
	void PresentSwapchain();
	void InitGBuffer();
	void ResizeGBuffer(UINT width, UINT height);
	void BuildGBufferResources();
	void BuildGBufferDescriptors();
	void DestroyResources();
	void InitImGuiInternal();


	inline LRESULT CALLBACK WndProcPrivate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{

		bool returnValue = false;

		// Need to wrap this in a mutex lock since this function will be called from a separate
		// thread than the render thread.
		mImGuiIoMutex.lock();
		returnValue = Dx12Device::IsImGuiInited() && ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);
		mImGuiIoMutex.unlock();

		return returnValue;

	}

	// This needs to be a recursive_mutex to make WndProcPrivate re-entrant safe
	std::recursive_mutex mImGuiIoMutex;

	bool mImGuiInited = false;


	static const int mSwapChainBufferCount = 3;
	
	Microsoft::WRL::ComPtr<ID3D12Device> mD3dDevice;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;

	/// TODO: This may not be the best way to handle this, but it works for now
	std::array< Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, mSwapChainBufferCount> mDirectCmdListAlloc;

	Microsoft::WRL::ComPtr<IDXGIFactory4> mDxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
	
	Microsoft::WRL::ComPtr<ID3D12Fence> mFence;

	

	

	
	int mCurrentBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[mSwapChainBufferCount];

	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	UINT mRtvHeapOffset = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	UINT mDsvHeapOffset = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvSrvUavHeap;
	UINT mCbvSrvUavHeapOffset = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mImGuiCbvSrvHeap;
	UINT mImGuiCbvSrvHeapOffset = 0;


	static const DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static const DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	UINT mRtvDescriptorSize;
	UINT mDsvDescriptorSize;
	UINT mCbvSrvUavDescriptorSize;

	// mCurrentFenceVal stores the global fence value, mFenceValues stores
	// the value of the global fence per frame at the time the frame was
	// presented. This way each frame only has to wait if the current fence
	// value reported by mFence->GetCompletedValue() is less than the frames
	// fence value
	UINT64 mCurrentFenceVal = 0;
	UINT64 mFenceValues[mSwapChainBufferCount];

	UINT mSwapchainWidth;
	UINT mSwapchainHeight;

	// Self contained GBuffer including heaps, resources, and descriptors
	GBuffer mGBuffer;
	HWND mWindow;
};

