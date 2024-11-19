#pragma once
#include "D3D12Common.h"

struct cbWorld
{
	DirectX::XMFLOAT4X4 mViewMat;
	DirectX::XMFLOAT4X4 mProjMat;
	DirectX::XMFLOAT3 mCameraPosition;
	//byte pad[128];
};

struct cbObject
{
	DirectX::XMFLOAT4X4 mWorldTransform;
	//byte pad[192];
};