#pragma once
#include "D3D12Common.h"
#include "Dx12Device.h"
#include "Buffer.h"
#include "ConstantBufferStructs.h"

class AppConstants
{
public:
	inline ID3D12Resource* GetResourceForFrame(UINT frameNum)
	{
		// Was returning the mResource, but its actually the upload
		// buffer that we use for updates, do we really even need
		// the main resource?
		return mFrameBuffer->Get(frameNum)->mUploadBuffer.Get();
	}

	inline void Init()
	{
		mFrameBuffer = Dx12Device::CreateFrameBuffer(&mWorldConstants, sizeof(cbWorld));
	}

	inline void UpdateBuffer(UINT bufferID)
	{
		auto buffer = mFrameBuffer->Get(bufferID);
		buffer->UpdateBuffer(0, sizeof(cbWorld), &mWorldConstants);

	}

	cbWorld mWorldConstants;
	std::unique_ptr<FrameBuffer> mFrameBuffer;
};

