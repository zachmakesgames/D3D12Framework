#include "RenderObject.h"


RenderObject::RenderObject(RenderObjectInit initValue)
{
	mMeshRef = initValue.meshRefName;
	mInstanced = initValue.isInstanced;
	mInstanceCount = initValue.instanceCount;
	mTextureRef = initValue.textureRefName;

	mConstants.isInstanced = mInstanced;
	mConstantBuffers = Dx12Device::GetDevice()->CreateFrameBuffer(&mConstants, sizeof(RenderItemConstants));

	if (mInstanced)
	{
		DirectX::XMMATRIX ident = DirectX::XMMatrixIdentity();
		DirectX::XMFLOAT4X4 identF;
		DirectX::XMStoreFloat4x4(&identF, ident);

		for (UINT i = 0; i < mInstanceCount; ++i)
		{
			RenderItemInstanceValues instVal = {};
			instVal.instanceTransform = identF;
			mInstanceValues.push_back(instVal);
		}

		mInstanceBuffers = Dx12Device::GetDevice()->CreateFrameBuffer(mInstanceValues.data(), sizeof(RenderItemInstanceValues) * mInstanceCount);
	}
}

RenderObject::~RenderObject()
{
	mConstantBuffers.reset();
	mInstanceBuffers.reset();
}