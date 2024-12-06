#pragma once
#include <DirectXMath.h>

namespace Physics
{
	class Triangle
	{
		public:
			Triangle();
			Triangle(DirectX::XMFLOAT3 vert1, DirectX::XMFLOAT3 vert2, DirectX::XMFLOAT3 vert3);
			bool DoesRayIntersect(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayDirection, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint = nullptr);
			bool DoesLineSegmentIntersect(DirectX::XMVECTOR point1, DirectX::XMVECTOR point2, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint = nullptr);

		private:
			DirectX::XMVECTOR mVertices[3];
			DirectX::XMVECTOR mSurfaceUnitNormal;
	};
}