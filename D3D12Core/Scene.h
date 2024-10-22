#pragma once
#include "Renderer/D3D12Common.h"
#include <vector>

struct ActorTransform
{
	DirectX::XMFLOAT3 mPosition;
	DirectX::XMFLOAT3 mRotation;
	DirectX::XMFLOAT3 mScale;
};

struct Actor
{
	std::string mMeshRef;
	std::string mTextureRef;
	UINT64 mInstanceCount;
	std::vector<ActorTransform> mTransforms;
};

struct Scene
{
	std::vector<Actor> mActors;
};