#pragma once
#include "D3D12Common.h"
#include "Dx12Device.h"
#include "Buffer.h"
#include "Mesh.h"

struct RenderItemConstants
{
	DirectX::XMFLOAT4X4 worldTransform;
};


class __declspec(dllexport) RenderObject
{
public:
	RenderObject(const std::string meshRefName);

	inline Buffer* GetBuffer(int bufferID)
	{
		return mConstantBuffers->Get(bufferID);
	}

	inline void UpdateBuffer(int bufferID)
	{
		auto buffer = mConstantBuffers->Get(bufferID);
		buffer->UpdateBuffer(0, sizeof(RenderItemConstants), &mConstants);

	}


	RenderItemConstants mConstants;
	std::string mMeshRef;

private:
	std::unique_ptr<FrameBuffer> mConstantBuffers;
	
};

