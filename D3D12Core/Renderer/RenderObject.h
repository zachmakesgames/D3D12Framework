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
	DirectX::XMFLOAT4 instanceColor;
};

struct RenderObjectInit
{
	const uint64_t meshRefName;
	const uint64_t textureRefName;
	const bool isInstanced;
	const UINT instanceCount;
};


class __declspec(dllexport) RenderObject
{
public:
	RenderObject(RenderObjectInit initValue);
	~RenderObject();

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

	inline UINT GetInstBufferSize()
	{
		return sizeof(RenderItemInstanceValues) * mInstanceCount;
	}


	RenderItemConstants mConstants;
	std::vector<RenderItemInstanceValues> mInstanceValues;
	uint64_t mMeshRef;
	uint64_t mTextureRef;
	UINT mInstanceCount;

private:
	bool mInstanced;
	std::unique_ptr<FrameBuffer> mConstantBuffers;
	std::unique_ptr<FrameBuffer> mInstanceBuffers;
	
};