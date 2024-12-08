#include "PhysicsAABB.h"

namespace Physics
{

	/// SECTION: Code examples from Real Time Collision Detection by Christer Ericson
	/// These methods were provided by Christer Ericson in his book Real Time Collision Detection,
	/// and adapted to work with DirectX Math.

	DirectX::XMFLOAT3 ClosestPointOnAABBToPoint(DirectX::XMFLOAT3 p, Rectangle aabb)
	{
		DirectX::XMFLOAT3 outPoint = DirectX::XMFLOAT3(0, 0, 0);
		{
			float v = p.x;
			if (v < aabb.mMin.x) v = aabb.mMin.x;
			if (v > aabb.mMax.x) v = aabb.mMax.x;
			outPoint.x = v;
		}
		{
			float v = p.y;
			if (v < aabb.mMin.y) v = aabb.mMin.y;
			if (v > aabb.mMax.y) v = aabb.mMax.y;
			outPoint.y = v;
		}
		{
			float v = p.z;
			if (v < aabb.mMin.z) v = aabb.mMin.z;
			if (v > aabb.mMax.z) v = aabb.mMax.z;
			outPoint.z = v;
		}

		return outPoint;
	}

	DirectX::XMFLOAT2 ClosestPointOnAABBToPoint(DirectX::XMFLOAT2 p, Rectangle aabb)
	{
		DirectX::XMFLOAT2 outPoint = DirectX::XMFLOAT2(0, 0);
		{
			float v = p.x;
			if (v < aabb.mMin.x) v = aabb.mMin.x;
			if (v > aabb.mMax.x) v = aabb.mMax.x;
			outPoint.x = v;
		}
		{
			float v = p.y;
			if (v < aabb.mMin.y) v = aabb.mMin.y;
			if (v > aabb.mMax.y) v = aabb.mMax.y;
			outPoint.y = v;
		}

		return outPoint;
	}

	bool DoesAABBOverlapSphereXYZ(Rectangle aabb, Sphere sphere, DirectX::XMFLOAT3* outPoint)
	{
		DirectX::XMFLOAT3 point;

		point = ClosestPointOnAABBToPoint(sphere.mCenter, aabb);
		if (outPoint != nullptr)
		{
			*outPoint = point;
		}


		DirectX::XMVECTOR pointV = DirectX::XMLoadFloat3(&point);
		DirectX::XMVECTOR centerV = DirectX::XMLoadFloat3(&sphere.mCenter);

		DirectX::XMVECTOR v = DirectX::XMVectorSubtract(pointV, centerV);

		DirectX::XMVECTOR vDot = DirectX::XMVector3Dot(v, v);
		DirectX::XMFLOAT3 dotResult;
		DirectX::XMStoreFloat3(&dotResult, vDot);

		float radiusSqr = sphere.mRadius * sphere.mRadius;

		return dotResult.x <= radiusSqr;
	}

	bool DoesAABBOverlapSphereXZ(Rectangle aabb, Sphere sphere, DirectX::XMFLOAT2* outPoint)
	{
		DirectX::XMFLOAT2 point;

		DirectX::XMFLOAT2 center = DirectX::XMFLOAT2(sphere.mCenter.x, sphere.mCenter.y);
		point = ClosestPointOnAABBToPoint(center, aabb);

		if (outPoint != nullptr)
		{
			*outPoint = point;
		}

		DirectX::XMVECTOR pointV = DirectX::XMLoadFloat2(&point);
		DirectX::XMVECTOR centerV = DirectX::XMLoadFloat2(&center);

		DirectX::XMVECTOR v = DirectX::XMVectorSubtract(pointV, centerV);

		DirectX::XMVECTOR vDot = DirectX::XMVector2Dot(v, v);
		DirectX::XMFLOAT2 dotResult;
		DirectX::XMStoreFloat2(&dotResult, vDot);

		float radiusSqr = sphere.mRadius * sphere.mRadius;

		return dotResult.x <= radiusSqr;
	}

	bool DoesRayIntersectAABBXYZ(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Rectangle aabb, float& tMin, DirectX::XMFLOAT3* outPoint)
	{
		tMin = -FLT_MAX;
		float tMax = FLT_MAX;

		float point3[3] = { point.x, point.y, point.z };
		float dir[3] = { direction.x, direction.y, direction.z };
		float aabbMin[3] = { aabb.mMin.x,aabb.mMin.y, aabb.mMin.z };
		float aabbMax[3] = { aabb.mMax.x,aabb.mMax.y, aabb.mMax.z };

		for (int i = 0; i < 3; ++i)
		{
			if (fabs(dir[i]) < 0.0001)
			{
				if (point3[i] < aabbMin[i] || point3[i] > aabbMax[i])
				{
					return false;
				}
			}
			else
			{
				float ood = 1.0f / dir[i];
				float t1 = (aabbMin[i] - point3[i]) * ood;
				float t2 = (aabbMax[i] - point3[i]) * ood;

				if (t1 > t2)
				{
					float t3 = t1;
					t1 = t2;
					t2 = t3;
				}

				if (t1 > tMin)
				{
					tMin = t1;
				}

				if (t2 < tMax)
				{
					tMax = t2;
				}

				if (tMin > tMax)
				{
					return false;
				}
			}
		}

		if (outPoint != nullptr)
		{
			DirectX::XMVECTOR pointV = DirectX::XMLoadFloat3(&point);
			DirectX::XMVECTOR directionV = DirectX::XMLoadFloat3(&direction);

			directionV = DirectX::XMVectorScale(directionV, tMin);
			DirectX::XMVECTOR result = DirectX::XMVectorAdd(pointV, directionV);
			
			DirectX::XMStoreFloat3(outPoint, result);
		}

		return true;
	}

	bool DoesRayIntersectAABBXZ(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 direction, Rectangle aabb, float& tMin, DirectX::XMFLOAT3* outPoint)
	{
		tMin = -FLT_MAX;
		float tMax = FLT_MAX;

		float point3[3] = { point.x, point.y, point.z };
		float dir[3] = { direction.x, direction.y, direction.z };
		float aabbMin[3] = { aabb.mMin.x,aabb.mMin.y, aabb.mMin.z };
		float aabbMax[3] = { aabb.mMax.x,aabb.mMax.y, aabb.mMax.z };

		for (int i = 0; i < 3; ++i)
		{
			if (fabs(dir[i]) < 0.0001)
			{
				if (point3[i] < aabbMin[i] || point3[i] > aabbMax[i])
				{
					if (i != 1)
					{
						// Special case for XZ, we skip the return of the Y component because
						// we only carea bout the X and Z
						return false;
					}
				}
			}
			else
			{
				float ood = 1.0f / dir[i];
				float t1 = (aabbMin[i] - point3[i]) * ood;
				float t2 = (aabbMax[i] - point3[i]) * ood;

				if (t1 > t2)
				{
					float t3 = t1;
					t1 = t2;
					t2 = t3;
				}

				if (t1 > tMin)
				{
					tMin = t1;
				}

				if (t2 < tMax)
				{
					tMax = t2;
				}

				if (tMin > tMax)
				{
					if (i != 1)
					{
						// Special case for XZ, we skip the return of the Y component because
						// we only carea bout the X and Z
						return false;
					}
				}
			}
		}

		if (outPoint != nullptr)
		{
			DirectX::XMVECTOR pointV = DirectX::XMLoadFloat3(&point);
			DirectX::XMVECTOR directionV = DirectX::XMLoadFloat3(&direction);

			directionV = DirectX::XMVectorScale(directionV, tMin);
			DirectX::XMVECTOR result = DirectX::XMVectorAdd(pointV, directionV);

			DirectX::XMStoreFloat3(outPoint, result);
		}

	}

	/// END SECTION: Code examples from Real Time Collision Detection by Christer Ericson
}