#pragma once
#include "D3D12Common.h"

class GBuffer
{
public:
	inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvHandle(UINT offset)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(mSrvHeap->GetCPUDescriptorHandleForHeapStart(), offset, mSrvDescriptorSize);
	}

	inline CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle(UINT offset)
	{
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(mSrvHeap->GetGPUDescriptorHandleForHeapStart(), offset, mSrvDescriptorSize);
	}

	inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuRtvHandle(UINT offset)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(mRtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, mRtvDescriptorSize);
	}

	inline ID3D12Resource* GetResource(int resourceIndex)
	{
		if (resourceIndex >= 0 || resourceIndex < 6)
		{
			return mGBuffer[resourceIndex].Get();
		}
		else
		{
			return nullptr;
		}
	}

	inline DXGI_FORMAT GetFormat()
	{
		return mFormat;
	}


private:
	UINT mWidth = 0;
	UINT mHeight = 0;

	DXGI_FORMAT mFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuRtv;

	UINT mRtvDescriptorSize;
	UINT mDsvDescriptorSize;
	UINT mSrvDescriptorSize;

	// Probably dont actually need this handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;
	
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 6> mGBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;

	// Probably dont actually need this heap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap = nullptr;

	friend class Dx12Device;
};

