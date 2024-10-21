#pragma once
#include "D3D12Common.h"
#include "Dx12Device.h"
#include "Buffer.h"
#include "Mesh.h"

struct RenderItemConstants
{
	DirectX::XMFLOAT4X4 worldTransform;
	bool isInstanced;
};

struct RenderItemInstanceValues
{
	DirectX::XMFLOAT4X4 instanceTransform;
};

struct RenderObjectInit
{
	const std::string meshRefName;
	const std::string textureRefName;
	const bool isInstanced;
	const UINT64 instanceCount;
};


class __declspec(dllexport) RenderObject
{
public:
	RenderObject(RenderObjectInit initValue);

	inline Buffer* GetBuffer(int bufferID)
	{
		return mConstantBuffers->Get(bufferID);
	}

	inline void UpdateBuffer(int bufferID)
	{
		auto buffer = mConstantBuffers->Get(bufferID);
		buffer->UpdateBuffer(0, sizeof(RenderItemConstants), &mConstants);
	}

	inline bool IsInstanced()
	{
		return mInstanced;
	}

	inline Buffer* GetInstanceBuffer(int bufferID)
	{
		return mInstanceBuffers->Get(bufferID);
	}

	inline void UpdateInstanceBuffer(int bufferID)
	{
		auto buffer = mInstanceBuffers->Get(bufferID);
		buffer->UpdateBuffer(0, sizeof(RenderItemInstanceValues) * mInstanceCount, mInstanceValues.data());
	}

	inline UINT64 GetInstBufferSize()
	{
		return sizeof(RenderItemInstanceValues) * mInstanceCount;
	}


	RenderItemConstants mConstants;
	std::vector<RenderItemInstanceValues> mInstanceValues;
	std::string mMeshRef;
	std::string mTextureRef;
	UINT64 mInstanceCount;

private:
	bool mInstanced;
	std::unique_ptr<FrameBuffer> mConstantBuffers;
	std::unique_ptr<FrameBuffer> mInstanceBuffers;
	
};