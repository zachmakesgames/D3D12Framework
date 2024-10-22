#pragma once

#include "D3D12Common.h"

class Buffer
{
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
	BYTE* mMappedData = nullptr;

	inline virtual ~Buffer()
	{
		mUploadBuffer->Unmap(0, nullptr);

		// Releasing the resources is causing crashes
	}

	inline void UpdateBuffer(int offset, int dataSize, void* data)
	{
		if (mMappedData != nullptr)
		{
			memcpy(mMappedData + offset, data, dataSize);
		}
	}

};

class FrameBuffer
{
public:
	
	// This is not the correct way to create an array
	// of unique pointers. Something like this:
	//std::unique_ptr<Buffer[]> mBuffers;
	std::unique_ptr<Buffer>* mBuffers;
	int mBufferCount = 0;

	inline virtual ~FrameBuffer()
	{	
		// need to free mBuffers, but we're not creating that array correctly
	}

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

