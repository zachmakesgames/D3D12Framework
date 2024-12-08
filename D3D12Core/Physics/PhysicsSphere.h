#pragma once
#include <DirectXMath.h>
#include "PhysicsPrimitives.h"

namespace Physics
{
	static bool DoesRayIntersectSphere(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Sphere sphere);
}