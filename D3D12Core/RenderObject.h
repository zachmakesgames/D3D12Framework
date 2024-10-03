#pragma once
#include "D3D12Common.h"
#include "Dx12Device.h"
#include "Buffer.h"

struct RenderItemConstants
{
	DirectX::XMFLOAT4X4 worldTransform;
};


class __declspec(dllexport) RenderObject
{
public:
	RenderObject();

	RenderItemConstants mConstants;

private:
	std::unique_ptr<FrameBuffer> mConstantBuffers;
};

