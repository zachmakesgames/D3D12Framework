#pragma once
#include "Triangle.h"
#include <memory>
#include <string>
#include "../Renderer/Vertex.h"
#include "PhysicsPrimitives.h"

namespace Physics
{
	class CollisionMesh
	{
		public:
			CollisionMesh();
			static std::unique_ptr<CollisionMesh> LoadMeshFromObj(std::string file_name);
			static std::unique_ptr<CollisionMesh> CreateMesh(Vertex* vertexData, UINT vertexCount);
			bool DoesRayIntersect(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayDirection, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint = nullptr);
			bool DoesLineSegmentIntersect(DirectX::XMVECTOR point1, DirectX::XMVECTOR point2, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint = nullptr);

			inline Rectangle GetAABB()
			{
				return mAABB;
			}

		private:
			UINT mTriangleCount = 0;
			Triangle* mTriangles = nullptr;
			Rectangle mAABB;
	};
}