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
		//this->modelName = objFileName;
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

		// 4 billion verts is probably fine lol
		UINT vertCount = 0;
		UINT normCount = 0;
		UINT texCount = 0;
		UINT triCount = 0;

		UINT maxTris = lines * 3;



		//Initialize the arrays


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

		int flop = 0;


		//fprintf(stderr, "Triangle counts in mesh %s: %i\n",this->modelName.c_str(),  triCount);
		//int dataLen = triCount * 3 + triCount * 3 + triCount *2;
		//int dataLen = triCount * 8 * 3;


		// Three vertices per triangle
		UINT dataLen = triCount * 3;
		returnMesh->mTriangles = new Triangle[triCount];
		returnMesh->mTriangleCount = triCount;


		UINT idx = 0;
		//GLfloat* data = new GLfloat[dataLen];
		//this->data = new vertex[dataLen];


		for (UINT i = 0; i < triCount; ++i) {
			int* tri = tris[i];
			//glm::ivec3* tri = tris[i];

			int v1 = tri[0];
			int v2 = tri[1];
			int v3 = tri[2];

			returnMesh->mTriangles[i] = Triangle(verts[v1 - 1], verts[v2 - 1], verts[v3 - 1]);
		}

		delete[] verts;
		delete[] tris;

		file.close();

		return std::move(returnMesh);
	}

	std::unique_ptr<CollisionMesh> CollisionMesh::CreateMesh(Vertex* vertexData, UINT vertexCount)
	{
		std::unique_ptr<CollisionMesh> returnMesh = std::make_unique<CollisionMesh>();
		int dataLen = (sizeof(vertexData) / sizeof(Vertex));

		if (dataLen % 3 != 0)
		{
			return nullptr;
		}
		else
		{
			int triCount = dataLen / 3;

			returnMesh->mTriangles = new Triangle[triCount];
			returnMesh->mTriangleCount = triCount;

			int idx = 0;
			for (int i = 0; i < triCount; ++i)
			{
				returnMesh->mTriangles[i] = Triangle(vertexData[idx++].position, vertexData[idx++].position, vertexData[idx++].position);
			}
		}

		return std::move(returnMesh);
	}


	bool CollisionMesh::DoesRayIntersect(DirectX::XMVECTOR rayOrigin, DirectX::XMVECTOR rayDirection, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint)
	{
		bool doesHit = false;

		for (int i = 0; i < mTriangleCount; ++i)
		{
			// TODO: accumulate hit points instead of just outputting the last one
			doesHit |= mTriangles[i].DoesRayIntersect(rayOrigin, rayDirection, transform, outHitPoint);
		}
		
		return doesHit;
	}

	bool CollisionMesh::DoesLineSegmentIntersect(DirectX::XMVECTOR point1, DirectX::XMVECTOR point2, DirectX::XMMATRIX transform, DirectX::XMFLOAT3* outHitPoint)
	{
		bool doesHit = false;

		for (int i = 0; i < mTriangleCount; ++i)
		{
			// TODO: accumulate hit points instead of just outputting the last one
			doesHit |= mTriangles[i].DoesLineSegmentIntersect(point1, point2, transform, outHitPoint);
		}

		return doesHit;
	}
}