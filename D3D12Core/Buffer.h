#pragma once

#include "D3D12Common.h"

class Buffer
{
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
};

class FrameBuffer
{
public:
	std::unique_ptr<Buffer>* mBuffers;
	int mBufferCount = 0;

	inline Buffer* Get(int bufferID)
	{
		if (bufferID < 0 || bufferID > mBufferCount)
		{
			return nullptr;
		}
		else
		{
			return mBuffers[bufferID].get();
		}
	}
};

