#pragma once
#include <DirectXMath.h>
#include "PhysicsPrimitives.h"

namespace Physics
{
	DirectX::XMFLOAT3 ClosestPointOnAABBToPoint(DirectX::XMFLOAT3 p, Rectangle aabb);
	DirectX::XMFLOAT2 ClosestPointOnAABBToPoint(DirectX::XMFLOAT2 p, Rectangle aabb);
	bool DoesAABBOverlapSphereXYZ(Rectangle aabb, Sphere sphere, DirectX::XMFLOAT3* outPoint = nullptr);
	bool DoesAABBOverlapSphereXZ(Rectangle aabb, Sphere sphere, DirectX::XMFLOAT2* outPoint = nullptr);
	bool DoesRayIntersectAABBXYZ(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Rectangle aabb, float& tMin, DirectX::XMFLOAT3* outPoint = nullptr);
	bool DoesRayIntersectAABBXYZ(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Rectangle aabb, DirectX::XMFLOAT4X4 aabbTransform, float& tMin, DirectX::XMFLOAT3* outPoint = nullptr);
	bool DoesRayIntersectAABBXZ(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Rectangle aabb, float& tMin, DirectX::XMFLOAT3* outPoint = nullptr);

	Rectangle RecalculateAABB(Rectangle aabb, DirectX::XMFLOAT4X4 aabbTransform);
}