#include "Dx12Device.h"


static Dx12Device* sDevice;

Dx12Device* Dx12Device::GetDevice()
{
	return sDevice;
}

Dx12Device::Dx12Device()
{

	// 1. Init debug controller if debug build
#if defined(DEBUG) || defined(_DEBUG)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	// 2. Create DXGI factory
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mDxgiFactory)));

	// 3. Create D3D12 device and fallback to WARP if necessary
	HRESULT result = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_12_1,
		IID_PPV_ARGS(&mD3dDevice));

	if (FAILED(result))
	{
		// Fall back to WARP device if a hardware device couldn't be created
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(mDxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&mD3dDevice)));
	}

	// 4. Create fence (fences?)
	ThrowIfFailed(mD3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

	// 5. Get and store descriptor sizes
	mRtvDescriptorSize = mD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = mD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = mD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// 6. Check quality support for necessary features
	// Couldn't get MSAA to work for some stupid reason, so we wont check support here

	D3D12_FEATURE_DATA_SHADER_MODEL requestedShaderModel = { D3D_SHADER_MODEL_6_6 };
	if (FAILED(mD3dDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &requestedShaderModel, sizeof(requestedShaderModel)))
		|| (requestedShaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_6))
		{
#if defined(DEBUG) || defined(_DEBUG)
			OutputDebugStringA("ERROR: Shader Model 6.6 is not supported!\r\n");
#endif
			ThrowIfFailed(E_FAIL);
		}
	else
	{
#if defined(DEBUG) || defined(_DEBUG)
		OutputDebugStringA("Shader Model 6.6 is supported\r\n");
#endif
	}

	// 7. Create command list objects
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(mD3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));

	for (int i = 0; i < mSwapChainBufferCount; ++i)
	{
		ThrowIfFailed(mD3dDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(mDirectCmdListAlloc[i].GetAddressOf())));
	}

	ThrowIfFailed(mD3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		mDirectCmdListAlloc[0].Get(),
		nullptr,
		IID_PPV_ARGS(mCommandList.GetAddressOf())));

	mCommandList->Close();

	// 8. Create descriptor heaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 256;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	ThrowIfFailed(mD3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 256;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	ThrowIfFailed(mD3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
	cbvSrvUavHeapDesc.NumDescriptors = 256;
	cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSrvUavHeapDesc.NodeMask = 0;

	ThrowIfFailed(mD3dDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(mCbvSrvUavHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC imGuiCbvSrvUavHeapDesc = {};
	imGuiCbvSrvUavHeapDesc.NumDescriptors = 256;
	imGuiCbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	imGuiCbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	imGuiCbvSrvUavHeapDesc.NodeMask = 0;

	ThrowIfFailed(mD3dDevice->CreateDescriptorHeap(&imGuiCbvSrvUavHeapDesc, IID_PPV_ARGS(mImGuiCbvSrvHeap.GetAddressOf())));

	// Set all the fence values to 0
	memset(mFenceValues, 0x00, sizeof(UINT64)* mSwapChainBufferCount);
}

void Dx12Device::ResizeSwapchain(int swapchainWidth, int swapchainHeight)
{
	if (mD3dDevice != nullptr && mSwapChain != nullptr && mDirectCmdListAlloc[mCurrentBackBuffer] != nullptr)
	{
		sDevice->mSwapchainWidth = swapchainWidth;
		sDevice->mSwapchainHeight = swapchainHeight;

		HardFlushCommandQueue();

		ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc[mCurrentBackBuffer].Get(), nullptr));

		for (int i = 0; i < mSwapChainBufferCount; ++i)
		{
			mSwapChainBuffer[i].Reset();
		}
		mDepthStencilBuffer.Reset();

		ThrowIfFailed(mSwapChain->ResizeBuffers(
			mSwapChainBufferCount,
			swapchainWidth, swapchainHeight,
			mBackBufferFormat,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		));

		mCurrentBackBuffer = 0;

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (int i = 0; i < mSwapChainBufferCount; ++i)
		{
			ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
			mD3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);

			rtvHeapHandle.Offset(1, mRtvDescriptorSize);
		}

		D3D12_RESOURCE_DESC depthStencilDesc = {};
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = swapchainWidth;
		depthStencilDesc.Height = swapchainHeight;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count = 1;			// TODO: Change this for MSAA
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE depthClear = {};
		depthClear.Format = mDepthStencilFormat;
		depthClear.DepthStencil.Depth = 1.0f;
		depthClear.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		ThrowIfFailed(mD3dDevice->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc, 
			D3D12_RESOURCE_STATE_COMMON,
			&depthClear,
			IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = mDepthStencilFormat;
		dsvDesc.Texture2D.MipSlice = 0;
		mD3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, mDsvHeap->GetCPUDescriptorHandleForHeapStart());

		CD3DX12_RESOURCE_BARRIER transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		mCommandList->ResourceBarrier(1, &transitionBarrier);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
		rtvHandle.Offset(mCurrentBackBuffer, mRtvDescriptorSize);

		ThrowIfFailed(mCommandList->Close());
		ID3D12CommandList* commandLists[] = { mCommandList.Get() };
		mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);


		ResizeGBuffer(swapchainWidth, swapchainHeight);
		HardFlushCommandQueue();
	}
	else
	{
		ThrowIfFailed(E_FAIL);
	}
}

void Dx12Device::FlushCommandQueue()
{
	if (mFence->GetCompletedValue() < mFenceValues[mCurrentBackBuffer])
	{

		++mCurrentFenceVal;

		mFenceValues[mCurrentBackBuffer] = mCurrentFenceVal;

		ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFenceVal));

		// Need to create this without the initial set flag, other the even is already signalled
		// LOL
		HANDLE waitHandle = CreateEventEx(nullptr, NULL, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFenceVal, waitHandle));

		WaitForSingleObject(waitHandle, INFINITE);
		CloseHandle(waitHandle);
	}
}

void Dx12Device::HardFlushCommandQueue()
{
	// We need to incrememnt the fence value and signal it
	// before checking it if we want to hard flush the queue.
	// This is necessary to allow the queue to flush before we
	// have rendered any frames but after we have put work on the
	// queue to create resources
	++mCurrentFenceVal;

	mFenceValues[mCurrentBackBuffer] = mCurrentFenceVal;

	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFenceVal));

	if (mFence->GetCompletedValue() < mFenceValues[mCurrentBackBuffer])
	{
		// Need to create this without the initial set flag, other the even is already signalled
		// LOL
		HANDLE waitHandle = CreateEventEx(nullptr, NULL, false, EVENT_ALL_ACCESS);

		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFenceVal, waitHandle));

		WaitForSingleObject(waitHandle, INFINITE);
		CloseHandle(waitHandle);
	}
}

void Dx12Device::PresentSwapchain()
{
	ThrowIfFailed(mSwapChain->Present(0, 0));
	++mCurrentFenceVal;

	mFenceValues[mCurrentBackBuffer] = mCurrentFenceVal;
	mCommandQueue->Signal(mFence.Get(), mCurrentFenceVal);

	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % mSwapChainBufferCount;

	// Advance the fence for the current frame resource when implemented
	// for now just advance the global fence

	//++mCurrentFenceVal;
	//mCommandQueue->Signal(mFence.Get(), mCurrentFenceVal);

}

void Dx12Device::Create()
{
	if (sDevice == nullptr)
	{
		sDevice = new Dx12Device();
	}
}

void Dx12Device::InitSwapchain(HWND window, int swapchainWidth, int swapchainHeight, bool windowed)
{
	if (sDevice != nullptr)
	{
		sDevice->mSwapchainWidth = swapchainWidth;
		sDevice->mSwapchainHeight = swapchainHeight;
		sDevice->mWindow = window;

		// Build the swap chain object
		sDevice->mSwapChain.Reset();

		DXGI_SWAP_CHAIN_DESC swapchainDesc = {};
		swapchainDesc.BufferDesc.Width = swapchainWidth;
		swapchainDesc.BufferDesc.Height = swapchainHeight;
		swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;	// TODO: Uncap this?
		swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapchainDesc.BufferDesc.Format = sDevice->mBackBufferFormat;
		swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapchainDesc.SampleDesc.Count = 1;		// TODO: Figure out how to get MSAA working
		swapchainDesc.SampleDesc.Quality = 0;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = sDevice->mSwapChainBufferCount;
		swapchainDesc.OutputWindow = window;
		swapchainDesc.Windowed = windowed;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


		HRESULT hr =(sDevice->mDxgiFactory->CreateSwapChain(
			sDevice->mCommandQueue.Get(),
			&swapchainDesc,
			sDevice->mSwapChain.GetAddressOf()
		));

		ThrowIfFailed(hr)

		// Handle the GBuffer resources while we're at it
		sDevice->InitGBuffer();
		sDevice->ResizeGBuffer(swapchainWidth, swapchainHeight);
		// Create back buffer and depth buffer resources
		sDevice->ResizeSwapchain(swapchainWidth, swapchainHeight);

	}
}

void Dx12Device::DestroySwapchain()
{
	if (sDevice != nullptr)
	{
		// ???????
	}
}

void Dx12Device::FlushQueue(bool hardFlush)
{
	if (sDevice != nullptr)
	{
		if (hardFlush)
		{
			sDevice->HardFlushCommandQueue();
		}
		else
		{
			sDevice->FlushCommandQueue();
		}
	}
}

ID3D12Resource* Dx12Device::GetCurrentBackBuffer()
{
	return sDevice->mSwapChainBuffer[sDevice->mCurrentBackBuffer].Get();
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> Dx12Device::CreateRootSignature(CD3DX12_ROOT_SIGNATURE_DESC* desc)
{
	if (sDevice != nullptr)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(desc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		Microsoft::WRL::ComPtr<ID3D12RootSignature> out_rootSig = nullptr;

		ThrowIfFailed(sDevice->mD3dDevice->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(out_rootSig.GetAddressOf())
		));

		return out_rootSig;
	}
	else
	{
		return nullptr;
	}
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Dx12Device::CreatePSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc)
{
	if (sDevice != nullptr)
	{
		Microsoft::WRL::ComPtr<ID3D12PipelineState> out_pso;
		ThrowIfFailed(sDevice->mD3dDevice->CreateGraphicsPipelineState(desc, IID_PPV_ARGS(&out_pso)));
		return out_pso;
	}
	else
	{
		return nullptr;
	}
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> Dx12Device::GetCommandList()
{
	if (sDevice != nullptr)
	{
		return sDevice->mCommandList;
	}
	else
	{
		return nullptr;
	}
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Dx12Device::GetCurrentFrameAllocator()
{
	if (sDevice != nullptr)
	{
		return sDevice->mDirectCmdListAlloc[sDevice->mCurrentBackBuffer];
	}
	else
	{
		return nullptr;
	}
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> Dx12Device::GetCommandQueue()
{
	if (sDevice != nullptr)
	{
		return sDevice->mCommandQueue;
	}
	else
	{
		return nullptr;
	}
}

void Dx12Device::Present()
{
	if (sDevice != nullptr)
	{
		sDevice->PresentSwapchain();
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12Device::GetCurrentBackBufferView()
{
	if (sDevice != nullptr)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			sDevice->mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
			sDevice->mCurrentBackBuffer,
			sDevice->mRtvDescriptorSize);
	}
	else
	{
		// Return a blank handle and pray
		return {};
	}
}

D3D12_CPU_DESCRIPTOR_HANDLE Dx12Device::GetDepthStencilView()
{
	if (sDevice != nullptr)
	{
		return sDevice->mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	}
	else
	{
		// Return a blank handle and pray
		return {};
	}
}

std::unique_ptr<Buffer> Dx12Device::CreateBuffer(const void* data, UINT64 bufferSize)
{
	UINT64 mod = bufferSize % 250;
	bufferSize += (250 - mod);

	auto newBuffer = std::make_unique<Buffer>();

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_HEAP_PROPERTIES heapPropertiesUpload(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	ThrowIfFailed(sDevice->mD3dDevice->CreateCommittedResource(
		&heapProperties, 
		D3D12_HEAP_FLAG_NONE, 
		&bufferDesc, 
		D3D12_RESOURCE_STATE_COMMON, 
		nullptr, 
		IID_PPV_ARGS(newBuffer->mResource.GetAddressOf())));

	ThrowIfFailed(sDevice->mD3dDevice->CreateCommittedResource(
		&heapPropertiesUpload,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(newBuffer->mUploadBuffer.GetAddressOf())));

	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = data;
	subResourceData.RowPitch = bufferSize;
	subResourceData.SlicePitch = bufferSize;

	CD3DX12_RESOURCE_BARRIER copyDestBarrier = CD3DX12_RESOURCE_BARRIER::Transition(newBuffer->mResource.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	sDevice->mCommandList->ResourceBarrier(1, &copyDestBarrier);

	UpdateSubresources<1>(
		sDevice->mCommandList.Get(),
		newBuffer->mResource.Get(), newBuffer->mUploadBuffer.Get(), 
		0, 0, 1, &subResourceData);

	CD3DX12_RESOURCE_BARRIER genericReadBarrier = CD3DX12_RESOURCE_BARRIER::Transition(newBuffer->mResource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	sDevice->mCommandList->ResourceBarrier(1, &genericReadBarrier);

	ThrowIfFailed(newBuffer->mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&(newBuffer->mMappedData))));

	return newBuffer;
}

std::unique_ptr<FrameBuffer> Dx12Device::CreateFrameBuffer(const void* data, UINT64 bufferSize)
{
	auto newFrameBuffer = std::make_unique<FrameBuffer>();
	int swapChainCount = sDevice->GetSwapchainBufferCount();
	newFrameBuffer->mBufferCount = swapChainCount;

	//newFrameBuffer->mBuffers = std::unique_ptr<Buffer[]>(swapChainCount);
	newFrameBuffer->mBuffers = new std::unique_ptr<Buffer>[swapChainCount];

	for (int i = 0; i < swapChainCount; ++i)
	{
		newFrameBuffer->mBuffers[i] = std::move(CreateBuffer(data, bufferSize));
	}

	return newFrameBuffer;
}

HRESULT Dx12Device::LoadTextureFromDDSFile(Texture* texture, UINT mostDetailedMip, FLOAT minLodClamp)
{
	HRESULT result = DirectX::CreateDDSTextureFromFile12(sDevice->mD3dDevice.Get(),
		sDevice->mCommandList.Get(), texture->mFileName.c_str(),
		texture->mResource, texture->mUploadHeap);

	if (!FAILED(result))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC textureSrvDesc = {};
		textureSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		textureSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		textureSrvDesc.Texture2D.MostDetailedMip = mostDetailedMip;
		textureSrvDesc.Texture2D.ResourceMinLODClamp = minLodClamp;
		textureSrvDesc.Format = texture->mResource->GetDesc().Format;
		textureSrvDesc.Texture2D.MipLevels = texture->mResource->GetDesc().MipLevels;

		texture->mSrvHandle = Dx12Device::CreateShaderResourceView(texture->mResource.Get(), &textureSrvDesc);
	}

	return result;
}

ResourceViewHandle Dx12Device::CreateRenderTargetView(ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* viewDesc)
{
	ResourceViewHandle handle = Dx12Device::GetNextRtvDescriptorHandle();
	sDevice->mD3dDevice->CreateRenderTargetView(resource, viewDesc, handle.mCpuHandle);

	return handle;
}

ResourceViewHandle Dx12Device::GetNextRtvDescriptorHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE nextHandle(sDevice->mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	nextHandle.Offset(sDevice->mRtvHeapOffset, sDevice->mRtvDescriptorSize);

	CD3DX12_GPU_DESCRIPTOR_HANDLE nextGpuHandle(sDevice->mRtvHeap->GetGPUDescriptorHandleForHeapStart());
	nextGpuHandle.Offset(sDevice->mRtvHeapOffset, sDevice->mRtvDescriptorSize);
	
	ResourceViewHandle handle;
	handle.mOffset = sDevice->mRtvHeapOffset;
	handle.mCpuHandle = nextHandle;
	handle.mGpuHandle = nextGpuHandle;

	sDevice->mRtvHeapOffset++;

	return handle;
}


ResourceViewHandle Dx12Device::CreateDepthStencilView(ID3D12Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC* viewDesc)
{
	ResourceViewHandle handle = Dx12Device::GetNextDsvDescriptorHandle();
	sDevice->mD3dDevice->CreateDepthStencilView(resource, viewDesc, handle.mCpuHandle);

	return handle;
}

ResourceViewHandle Dx12Device::GetNextDsvDescriptorHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE nextHandle(sDevice->mDsvHeap->GetCPUDescriptorHandleForHeapStart());
	nextHandle.Offset(sDevice->mDsvHeapOffset, sDevice->mDsvDescriptorSize);

	CD3DX12_GPU_DESCRIPTOR_HANDLE nextGpuHandle(sDevice->mDsvHeap->GetGPUDescriptorHandleForHeapStart());
	nextGpuHandle.Offset(sDevice->mDsvHeapOffset, sDevice->mDsvDescriptorSize);

	ResourceViewHandle handle;
	handle.mOffset = sDevice->mDsvHeapOffset;
	handle.mCpuHandle = nextHandle;
	handle.mGpuHandle = nextGpuHandle;

	sDevice->mDsvHeapOffset++;

	return handle;
}

ResourceViewHandle Dx12Device::CreateConstantBufferView(D3D12_CONSTANT_BUFFER_VIEW_DESC* viewDesc)
{
	ResourceViewHandle handle = Dx12Device::GetNextCbvSrvUavDescriptorHandle();
	sDevice->mD3dDevice->CreateConstantBufferView(viewDesc, handle.mCpuHandle);

	return handle;
}

ResourceViewHandle Dx12Device::CreateShaderResourceView(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC* viewDesc)
{
	ResourceViewHandle handle = Dx12Device::GetNextCbvSrvUavDescriptorHandle();
	sDevice->mD3dDevice->CreateShaderResourceView(resource, viewDesc, handle.mCpuHandle);

	return handle;
}
ResourceViewHandle Dx12Device::CreateUnorderedAccessView(ID3D12Resource* resource, ID3D12Resource* counterResource, D3D12_UNORDERED_ACCESS_VIEW_DESC* viewDesc)
{
	ResourceViewHandle handle = Dx12Device::GetNextCbvSrvUavDescriptorHandle();
	sDevice->mD3dDevice->CreateUnorderedAccessView(resource, counterResource, viewDesc, handle.mCpuHandle);

	return handle;
}


ResourceViewHandle Dx12Device::GetNextCbvSrvUavDescriptorHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE nextHandle(sDevice->mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
	nextHandle.Offset(sDevice->mCbvSrvUavHeapOffset, sDevice->mCbvSrvUavDescriptorSize);

	ResourceViewHandle outHandle = {};
	outHandle.mCpuHandle = nextHandle;
	outHandle.mOffset = sDevice->mCbvSrvUavHeapOffset;

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(sDevice->mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
	gpuHandle.Offset(outHandle.mOffset, sDevice->mCbvSrvUavDescriptorSize);
	outHandle.mGpuHandle = gpuHandle;
	sDevice->mCbvSrvUavHeapOffset++;

	return outHandle;
}

const std::array<ID3D12DescriptorHeap*, 4> Dx12Device::GetDescriptorHeaps()
{
	return { sDevice->mRtvHeap.Get(), 
		sDevice->mDsvHeap.Get(), 
		sDevice->mCbvSrvUavHeap.Get(), 
		sDevice->mImGuiCbvSrvHeap.Get()};
}

void Dx12Device::InitGBuffer()
{
	// Create specific descriptor heaps for the GBuffer so that only live resources and the back buffers
	// live on the main heaps. The main heaps may need to be reset when changing scenes but the GBuffer heaps
	// should stay the same for the lifetime of the app
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 6;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;


	ThrowIfFailed(mD3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mGBuffer.mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 6;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	ThrowIfFailed(mD3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mGBuffer.mDsvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
	cbvSrvUavHeapDesc.NumDescriptors = 6;
	cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSrvUavHeapDesc.NodeMask = 0;

	ThrowIfFailed(mD3dDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(mGBuffer.mSrvHeap.GetAddressOf())));

	mGBuffer.mDsvDescriptorSize = mDsvDescriptorSize;
	mGBuffer.mRtvDescriptorSize = mRtvDescriptorSize;
	mGBuffer.mSrvDescriptorSize = mCbvSrvUavDescriptorSize;
}

void Dx12Device::ResizeGBuffer(UINT width, UINT height)
{
	// Create the committed resources for each gbuffer RT
	mGBuffer.mWidth = width;
	mGBuffer.mHeight = height;

	BuildGBufferResources();
	BuildGBufferDescriptors();
}

void Dx12Device::BuildGBufferResources()
{
	for (int i = 0; i < 6; ++i)
	{
		mGBuffer.mGBuffer[i].Reset();
	}


	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mGBuffer.mWidth;
	texDesc.Height = mGBuffer.mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mGBuffer.mFormat;
	// TODO: Add MSAA stuff here
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags =
		(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mGBuffer.mFormat;
	optClear.Color[0] = mGBuffer.mClearColor[0];
	optClear.Color[1] = mGBuffer.mClearColor[1];
	optClear.Color[2] = mGBuffer.mClearColor[2];
	optClear.Color[3] = mGBuffer.mClearColor[3];

	CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	for (int i = 0; i < 6; ++i)
	{
		ThrowIfFailed(mD3dDevice->CreateCommittedResource(
			&defaultHeap,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			&optClear,
			IID_PPV_ARGS(&mGBuffer.mGBuffer[i])));
	}
}

void Dx12Device::BuildGBufferDescriptors()
{
	// Create the resource view descriptors
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = mGBuffer.mFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.Texture2D.PlaneSlice = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mGBuffer.mSrvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < 6; ++i)
	{
		mD3dDevice->CreateShaderResourceView(mGBuffer.mGBuffer[i].Get(), &srvDesc, cpuSrvHandle);
		cpuSrvHandle.Offset(1, mCbvSrvUavDescriptorSize);
	}

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = mGBuffer.mFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuRtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mGBuffer.mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (int i = 0; i < 6; ++i)
	{
		mD3dDevice->CreateRenderTargetView(mGBuffer.mGBuffer[i].Get(), &rtvDesc, cpuRtvHandle);
		cpuRtvHandle.Offset(1, mRtvDescriptorSize);
	}
}

GBuffer* Dx12Device::GetGBuffer()
{
	if (sDevice != nullptr)
	{
		return &sDevice->mGBuffer;
	}
	else
	{
		return nullptr;
	}
}

D3D12_VIEWPORT Dx12Device::GetViewport()
{
	D3D12_VIEWPORT viewport;
	if (sDevice != nullptr)
	{
		viewport = { 0, 0, (float)sDevice->mSwapchainWidth, (float)sDevice->mSwapchainHeight, 0, 1 };
	}
	else
	{
		viewport = { 0, 0, 0, 0, 0, 0 };
	}

	return viewport;
}

void Dx12Device::Resize(int width, int height)
{
	if (sDevice != nullptr)
	{
		sDevice->ResizeSwapchain(width, height);
	}
}

void Dx12Device::Destroy()
{
	if (sDevice != nullptr)
	{
		sDevice->DestroyResources();
	}
}

void Dx12Device::DestroyResources()
{
	// Clean up our blocks on the floor

	HardFlushCommandQueue();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	mGBuffer.Destroy();

	for (int i = 0; i < mSwapChainBufferCount; ++i)
	{
		mSwapChainBuffer[i].Reset();
		mDirectCmdListAlloc[i].Reset();
	}

	mDepthStencilBuffer.Reset();
	mCbvSrvUavHeap.Reset();
	mDsvHeap.Reset();
	mRtvHeap.Reset();
	mCommandList.Reset();
	mCommandQueue.Reset();
	mFence.Reset();
	mSwapChain.Reset();
	mD3dDevice.Reset();
	mDxgiFactory.Reset();
}

void Dx12Device::InitImGui()
{
	if (sDevice != nullptr)
	{
		sDevice->InitImGuiInternal();
	}
}

void Dx12Device::InitImGuiInternal()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplWin32_Init(mWindow);
	ImGui_ImplDX12_Init(mD3dDevice.Get(), mSwapChainBufferCount,
		mBackBufferFormat, mImGuiCbvSrvHeap.Get(),
		mImGuiCbvSrvHeap->GetCPUDescriptorHandleForHeapStart(),
		mImGuiCbvSrvHeap->GetGPUDescriptorHandleForHeapStart());

	mImGuiInited = true;

}

bool Dx12Device::IsImGuiInited()
{
	if (sDevice != nullptr)
	{
		return sDevice->mImGuiInited;
	}
	else
	{
		return false;
	}
}

LRESULT CALLBACK Dx12Device::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (sDevice != nullptr)
	{
		return sDevice->WndProcPrivate(hWnd, message, wParam, lParam);
	}
	else
	{
		return -1;
	}
}

std::recursive_mutex* Dx12Device::GetImGuiIoMutex()
{
	if (sDevice != nullptr)
	{
		return &sDevice->mImGuiIoMutex;
	}
	else
	{
		return nullptr;
	}
}