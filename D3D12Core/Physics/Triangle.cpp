#include "Triangle.h"

namespace Physics
{
	Triangle::Triangle()
	{
		DirectX::XMFLOAT3 zero = DirectX::XMFLOAT3(0, 0, 0);
		mVertices[0] = DirectX::XMLoadFloat3(&zero);
		mVertices[1] = DirectX::XMLoadFloat3(&zero);
		mVertices[2] = DirectX::XMLoadFloat3(&zero);
		mSurfaceUnitNormal = DirectX::XMLoadFloat3(&zero);
	}

	Triangle::Triangle(DirectX::XMFLOAT3 vert1, DirectX::XMFLOAT3 vert2, DirectX::XMFLOAT3 vert3)
	{
		mVertices[0] = DirectX::XMLoadFloat3(&vert1);
		mVertices[1] = DirectX::XMLoadFloat3(&vert2);
		mVertices[2] = DirectX::XMLoadFloat3(&vert3);

		



		DirectX::XMVECTOR a = DirectX::XMVectorSubtract(mVertices[2], mVertices[1]);
		DirectX::XMVECTOR b = DirectX::XMVectorSubtract(mVertices[1], mVertices[0]);

		mSurfaceUnitNormal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(a, b));
	}

	bool Triangle::DoesRayIntersect(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayDirection, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint)
	{
		// Surely there must be a better way to do this...
		// There is, and don't call me Shirly
		DirectX::XMFLOAT3X3 normalTransform;
		DirectX::XMStoreFloat3x3(&normalTransform, transform);
		DirectX::XMMATRIX normalTransformMat = DirectX::XMLoadFloat3x3(&normalTransform);

		DirectX::XMVECTOR transformVerts[3];
		transformVerts[0] = DirectX::XMVector3Transform(mVertices[0], transform);
		transformVerts[1] = DirectX::XMVector3Transform(mVertices[1], transform);
		transformVerts[2] = DirectX::XMVector3Transform(mVertices[2], transform);

		DirectX::XMVECTOR normal = DirectX::XMVector3Transform(mSurfaceUnitNormal, normalTransformMat);

		// All this to get a simple dot product
		DirectX::XMVECTOR dotProductIntermediateV = DirectX::XMVector3Dot(rayDirection, normal);
		DirectX::XMFLOAT3 dotProductIntermediate;
		DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);
		
		float rayDirDotNormal = dotProductIntermediate.x;
		
		if (fabs(rayDirDotNormal) < 0.0001)
		{
			// Dot product of ray and normal are very nearly 0, so no hit
			return false;
		}

		dotProductIntermediateV = DirectX::XMVector3Dot(transformVerts[0], normal);
		DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);
		
		float pointDotNorm = dotProductIntermediate.x;

		dotProductIntermediateV = DirectX::XMVector3Dot(rayOrigin, normal);
		DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

		float rayOriginDotNormal = dotProductIntermediate.x;

		float t = (pointDotNorm - rayOriginDotNormal) / rayDirDotNormal;

		if (t < 0.0001)
		{
			// if t is negative, the hit is behind the ray
			return false;
		}
		else
		{
			DirectX::XMVECTOR rayDirScale = DirectX::XMVectorScale(rayDirection, t);
			DirectX::XMVECTOR hitPoint = DirectX::XMVectorAdd(rayOrigin, rayDirScale);

			DirectX::XMVECTOR r1 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(hitPoint, transformVerts[0]), DirectX::XMVectorSubtract(transformVerts[1], transformVerts[0]));
			DirectX::XMVECTOR r2 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(hitPoint, transformVerts[1]), DirectX::XMVectorSubtract(transformVerts[2], transformVerts[1]));
			DirectX::XMVECTOR r3 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(hitPoint, transformVerts[2]), DirectX::XMVectorSubtract(transformVerts[0], transformVerts[2]));

			dotProductIntermediateV = DirectX::XMVector3Dot(normal, r1);
			DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

			float d1 = dotProductIntermediate.x;

			dotProductIntermediateV = DirectX::XMVector3Dot(normal, r2);
			DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

			float d2 = dotProductIntermediate.x;

			dotProductIntermediateV = DirectX::XMVector3Dot(normal, r3);
			DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

			float d3 = dotProductIntermediate.x;

			if (d1 >= 0.0 && d2 >= 0.0 && d3 >= 0.0)
			{
				if (outHitPoint != nullptr)
				{
					DirectX::XMStoreFloat3(outHitPoint, hitPoint);
				}

				return true;
			}
			else
			{
				return false;
			}
		}
		
	}

	bool Triangle::DoesLineSegmentIntersect(DirectX::XMVECTOR point1, DirectX::XMVECTOR point2, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint)
	{
		// Surely there must be a better way to do this...
		// There is, and don't call me Shirly
		DirectX::XMFLOAT3X3 normalTransform;
		DirectX::XMStoreFloat3x3(&normalTransform, transform);
		DirectX::XMMATRIX normalTransformMat = DirectX::XMLoadFloat3x3(&normalTransform);

		DirectX::XMVECTOR transformVerts[3];
		transformVerts[0] = DirectX::XMVector3Transform(mVertices[0], transform);
		transformVerts[1] = DirectX::XMVector3Transform(mVertices[1], transform);
		transformVerts[2] = DirectX::XMVector3Transform(mVertices[2], transform);

		DirectX::XMVECTOR normal = DirectX::XMVector3Transform(mSurfaceUnitNormal, normalTransformMat);

		DirectX::XMVECTOR rayDirection = DirectX::XMVectorSubtract(point2, point1);

		// All this to get a simple dot product
		DirectX::XMVECTOR dotProductIntermediateV = DirectX::XMVector3Dot(rayDirection, normal);
		DirectX::XMFLOAT3 dotProductIntermediate;
		DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

		float rayDirDotNormal = dotProductIntermediate.x;

		if (fabs(rayDirDotNormal) < 0.0001)
		{
			// Dot product of ray and normal are very nearly 0, so no hit
			return false;
		}

		dotProductIntermediateV = DirectX::XMVector3Dot(transformVerts[0], normal);
		DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

		float pointDotNorm = dotProductIntermediate.x;

		dotProductIntermediateV = DirectX::XMVector3Dot(point1, normal);
		DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

		float rayOriginDotNormal = dotProductIntermediate.x;

		float t = (pointDotNorm - rayOriginDotNormal) / rayDirDotNormal;

		if (t < 0.0001 || t > 1.0f)
		{
			// if t is negative, the hit is behind the ray
			return false;
		}
		else
		{
			DirectX::XMVECTOR rayDirScale = DirectX::XMVectorScale(rayDirection, t);
			DirectX::XMVECTOR hitPoint = DirectX::XMVectorAdd(point1, rayDirScale);

			DirectX::XMVECTOR r1 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(hitPoint, transformVerts[0]), DirectX::XMVectorSubtract(transformVerts[1], transformVerts[0]));
			DirectX::XMVECTOR r2 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(hitPoint, transformVerts[1]), DirectX::XMVectorSubtract(transformVerts[2], transformVerts[1]));
			DirectX::XMVECTOR r3 = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(hitPoint, transformVerts[2]), DirectX::XMVectorSubtract(transformVerts[0], transformVerts[2]));

			dotProductIntermediateV = DirectX::XMVector3Dot(normal, r1);
			DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

			float d1 = dotProductIntermediate.x;

			dotProductIntermediateV = DirectX::XMVector3Dot(normal, r2);
			DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

			float d2 = dotProductIntermediate.x;

			dotProductIntermediateV = DirectX::XMVector3Dot(normal, r3);
			DirectX::XMStoreFloat3(&dotProductIntermediate, dotProductIntermediateV);

			float d3 = dotProductIntermediate.x;

			if (d1 >= 0.0 && d2 >= 0.0 && d3 >= 0.0)
			{
				if (outHitPoint != nullptr)
				{
					DirectX::XMStoreFloat3(outHitPoint, hitPoint);
				}

				return true;
			}
			else
			{
				return false;
			}
		}

	}
}