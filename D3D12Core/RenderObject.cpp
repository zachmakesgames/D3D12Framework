#include "RenderObject.h"

RenderObject::RenderObject()
{
	mConstantBuffers = Dx12Device::GetDevice()->CreateFrameBuffer(&mConstants, sizeof(RenderItemConstants));
}
