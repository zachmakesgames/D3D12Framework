#pragma once
#include <DirectXMath.h>

namespace Physics
{
	struct Sphere
	{
		float mRadius;
		DirectX::XMFLOAT3 mCenter;
	};

	struct Rectangle
	{
		DirectX::XMFLOAT3 mMin;
		DirectX::XMFLOAT3 mMax;
	};

	struct Ray
	{
		DirectX::XMFLOAT3 mPosition;
		DirectX::XMFLOAT3 mDirection;
	};
}