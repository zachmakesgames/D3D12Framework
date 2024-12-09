#include "CollisionMesh.h"
#include <string>
#include <iostream>
#include <fstream>

namespace Physics
{
	CollisionMesh::CollisionMesh()
	{

	}

	std::unique_ptr<CollisionMesh> CollisionMesh::LoadMeshFromObj(std::string file_name)
	{
		std::unique_ptr<CollisionMesh> returnMesh = std::make_unique<CollisionMesh>();
		std::ifstream file;
		file.open(file_name, std::fstream::in);
		if (!file) {
			OutputDebugStringA("ERROR: Tried to load a mesh from file, but file was not found!");
			return std::move(returnMesh);
		}
		int lines = 0;
		std::string line = "";
		while (std::getline(file, line)) {
			++lines;
		}
		file.clear();
		file.seekg(0);

		UINT vertCount = 0;
		UINT normCount = 0;
		UINT texCount = 0;
		UINT triCount = 0;

		UINT maxTris = lines * 3;



		//Initialize the array
		DirectX::XMFLOAT3* verts = new DirectX::XMFLOAT3[lines];

		int **tris = new int*[lines];

		for (int i = 0; i < lines; ++i) {
			tris[i] = new int[3];
		}

		//end init of arrays
		while (std::getline(file, line)) {
			if ((line[0] == 'v' || line[0] == 'V') && line[1] == ' ') {
				float x, y, z;
				sscanf_s(line.c_str(), "v %f %f %f\n", &x, &y, &z);
				verts[vertCount].x = x;
				verts[vertCount].y = y;
				verts[vertCount].z = z;
				++vertCount;
			}
			if (line[0] == 'f') {
				int vertsPerFace = 0;
				for (int i = 0; i < strlen(line.c_str()); ++i) {
					if (line[i] == '/') {
						++vertsPerFace;
					}
				}
				vertsPerFace /= 2;

				int v1, v2, v3, v4;
				int t1, t2, t3, t4;
				int n1, n2, n3, n4;
				v1 = 0;
				v2 = 0;
				v3 = 0;
				v4 = 0;
				t1 = 0;
				t2 = 0;
				t3 = 0;
				t4 = 0;
				n1 = 0;
				n2 = 0;
				n3 = 0;
				n4 = 0;


				if (vertsPerFace == 3) {
					//fix for no texture coords
					std::size_t pos = line.find("//");
					while (pos != std::string::npos) {
						line.insert(pos + 1, "0");
						pos = line.find("//");
					}
					//load in the triangle
					sscanf_s(line.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
					if (triCount == maxTris) {
						//need to resize tris
						//gonna leave this out for now, heh......

					}

					tris[triCount][0] = v1;
					tris[triCount][1] = v2;
					tris[triCount][2] = v3;
					++triCount;


				}
				if (vertsPerFace == 4) {
					//fix for no texture coords
					std::size_t pos = line.find("//");
					while (pos != std::string::npos) {
						line.insert(pos + 1, "0");
						pos = line.find("//");
					}
					//load in the quad and try to triangulate
					sscanf_s(line.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3, &v4, &t4, &n4);
					if (triCount == maxTris) {
						//need to resize tris
					}


					tris[triCount][0] = v1;
					tris[triCount][1] = v2;
					tris[triCount][2] = v3;
					++triCount;

					tris[triCount][0] = v3;
					tris[triCount][1] = v4;
					tris[triCount][2] = v1;
					++triCount;
				}
			}
		}


		// Three vertices per triangle
		UINT dataLen = triCount * 3;
		returnMesh->mTriangles = new Triangle[triCount];
		returnMesh->mTriangleCount = triCount;


		UINT idx = 0;

		float xMin = FLT_MAX;
		float xMax = -FLT_MAX;
		float yMin = FLT_MAX;
		float yMax = -FLT_MAX;
		float zMin = FLT_MAX;
		float zMax = -FLT_MAX;


		for (UINT i = 0; i < triCount; ++i) {
			int* tri = tris[i];

			DirectX::XMFLOAT3 v[3] = { verts[tri[0] - 1], verts[tri[1] - 1], verts[tri[2] - 1] };

			// Compute an AABB while we're at it
			for (int i = 0; i < 3; ++i)
			{
				DirectX::XMFLOAT3 vert = v[i];

				// Compute min/max for each component
				xMin = fmin(xMin, vert.x);
				xMax = fmax(xMax, vert.x);

				yMin = fmin(yMin, vert.y);
				yMax = fmax(yMax, vert.y);

				zMin = fmin(zMin, vert.z);
				zMax = fmax(zMax, vert.z);
			}


			returnMesh->mTriangles[i] = Triangle(v[0], v[1], v[2]);
			returnMesh->mAABB.mMin = DirectX::XMFLOAT3(xMin, yMin, zMin);
			returnMesh->mAABB.mMax = DirectX::XMFLOAT3(xMax, yMax, zMax);
		}

		delete[] verts;
		delete[] tris;

		file.close();

		return std::move(returnMesh);
	}

	std::unique_ptr<CollisionMesh> CollisionMesh::CreateMesh(Vertex* vertexData, UINT vertexCount)
	{
		std::unique_ptr<CollisionMesh> returnMesh = std::make_unique<CollisionMesh>();

		// If vertex count not divisible by 3 then the mesh is not a triangulated mesh and
		// we wont be able to easily create the colision mesh, so just return a null pointer
		if (vertexCount % 3 != 0)
		{
			return nullptr;
		}
		else
		{
			int triCount = vertexCount / 3;

			returnMesh->mTriangles = new Triangle[triCount];
			returnMesh->mTriangleCount = triCount;

			float xMin = FLT_MAX;
			float xMax = -FLT_MAX;
			float yMin = FLT_MAX;
			float yMax = -FLT_MAX;
			float zMin = FLT_MAX;
			float zMax = -FLT_MAX;

			int idx = 0;
			for (int i = 0; i < triCount; ++i)
			{
				DirectX::XMFLOAT3 v[3] = { vertexData[idx++].position, vertexData[idx++].position, vertexData[idx++].position };
				// Compute an AABB while we're at it
				for (int i = 0; i < 3; ++i)
				{
					DirectX::XMFLOAT3 vert = v[i];

					// Compute min/max for each component
					xMin = fmin(xMin, vert.x);
					xMax = fmax(xMax, vert.x);

					yMin = fmin(yMin, vert.y);
					yMax = fmax(yMax, vert.y);

					zMin = fmin(zMin, vert.z);
					zMax = fmax(zMax, vert.z);
				}

				returnMesh->mTriangles[i] = Triangle(v[0], v[1], v[2]);
			}
			returnMesh->mAABB.mMin = DirectX::XMFLOAT3(xMin, yMin, zMin);
			returnMesh->mAABB.mMax = DirectX::XMFLOAT3(xMax, yMax, zMax);
		}

		return std::move(returnMesh);
	}


	bool CollisionMesh::DoesRayIntersect(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayDirection, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint)
	{
		bool doesHit = false;

		for (UINT i = 0; i < mTriangleCount; ++i)
		{
			// TODO: accumulate hit points instead of just outputting the last one
			doesHit |= mTriangles[i].DoesRayIntersect(rayOrigin, rayDirection, transform, outHitPoint);
		}
		
		return doesHit;
	}

	bool CollisionMesh::DoesLineSegmentIntersect(DirectX::XMVECTOR point1, DirectX::XMVECTOR point2, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint)
	{
		bool doesHit = false;

		for (UINT i = 0; i < mTriangleCount; ++i)
		{
			// TODO: accumulate hit points instead of just outputting the last one
			doesHit |= mTriangles[i].DoesLineSegmentIntersect(point1, point2, transform, outHitPoint);
		}

		return doesHit;
	}
}