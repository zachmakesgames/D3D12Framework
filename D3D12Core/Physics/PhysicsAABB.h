#pragma once
#include <DirectXMath.h>
#include "PhysicsPrimitives.h"

namespace Physics
{
	static DirectX::XMFLOAT3 ClosestPointOnAABBToPoint(DirectX::XMFLOAT3 p, Rectangle aabb);
	static DirectX::XMFLOAT2 ClosestPointOnAABBToPoint(DirectX::XMFLOAT2 p, Rectangle aabb);
	static bool DoesAABBOverlapSphereXYZ(Rectangle aabb, Sphere sphere, DirectX::XMFLOAT3* outPoint = nullptr);
	static bool DoesAABBOverlapSphereXZ(Rectangle aabb, Sphere sphere, DirectX::XMFLOAT2* outPoint = nullptr);
	static bool DoesRayIntersectAABBXYZ(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Rectangle aabb, float& tMin, DirectX::XMFLOAT3* outPoint = nullptr);
	static bool DoesRayIntersectAABBXZ(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Rectangle aabb, float& tMin, DirectX::XMFLOAT3* outPoint = nullptr);
}