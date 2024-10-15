#include "RenderObject.h"

RenderObject::RenderObject(const std::string meshRefName)
{
	mConstantBuffers = Dx12Device::GetDevice()->CreateFrameBuffer(&mConstants, sizeof(RenderItemConstants));
	mMeshRef = meshRefName;
}
