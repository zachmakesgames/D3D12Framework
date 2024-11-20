#pragma once
#include "D3D12Common.h"

class GBuffer
{
public:
	inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrvHandle(UINT offset)
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(mSrvHeap->GetCPUDescriptorHandleForHeapStart(), offset, mSrvDescriptorSize);
	}

	inline D3D12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle()
	{
		return mSrvHeap->GetGPUDescriptorHandleForHeapStart();
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

	inline ID3D12DescriptorHeap* GetSrvHeap()
	{
		return mSrvHeap.Get();
	}

	inline std::array<float, 4> GetClearColor()
	{
		return mClearColor;
	}

	inline void Destroy()
	{
		for (int i = 0; i < 6; ++i)
		{
			mGBuffer[i].Reset();
		}
		mRtvHeap.Reset();
		mSrvHeap.Reset();
		mDsvHeap.Reset();
	}

private:
	UINT mWidth = 0;
	UINT mHeight = 0;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mSrvDescriptorSize = 0;

	DXGI_FORMAT mFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuRtv;

	// Probably dont actually need this handle
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;
	
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 6> mGBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mSrvHeap = nullptr;

	// Probably dont actually need this heap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap = nullptr;

	const std::array<float, 4> mClearColor = { 0.2f, 0.2f, 0.5f, 1.f };

	friend class Dx12Device;
};

