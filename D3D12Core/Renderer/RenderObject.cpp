#include "RenderObject.h"

RenderObject::RenderObject(const std::string meshRefName):
	mMeshRef(meshRefName)
{
	mConstantBuffers = Dx12Device::GetDevice()->CreateFrameBuffer(&mConstants, sizeof(RenderItemConstants));
}
