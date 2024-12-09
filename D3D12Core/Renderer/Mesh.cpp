#include "Mesh.h"

#include <string>
#include <iostream>
#include <fstream>

std::unique_ptr<Mesh> Mesh::LoadMeshFromObj(std::string file_name)
{
	std::unique_ptr<Mesh> returnMesh = std::make_unique<Mesh>();
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



	//Initialize the arrays
	DirectX::XMFLOAT3* verts = new DirectX::XMFLOAT3[lines];
	DirectX::XMFLOAT3* norms = new DirectX::XMFLOAT3[lines];
	DirectX::XMFLOAT2* tex = new DirectX::XMFLOAT2[lines];

	DirectX::XMINT3** tris = new DirectX::XMINT3 * [lines];
	for (int i = 0; i < lines; ++i) {
		//Using an XMINT3 istead of a XMFLOAT3 to avoid roundoff errors and compiler warnings
		tris[i] = new DirectX::XMINT3[3];//using glm::vec3 creatively here. 
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
		if (line[0] == 'v' && line[1] == 'n') {
			float xn, yn, zn;
			sscanf_s(line.c_str(), "vn %f %f %f\n", &xn, &yn, &zn);
			norms[normCount].x = xn;
			norms[normCount].y = yn;
			norms[normCount].z = zn;
			++normCount;
		}
		if (line[0] == 'v' && line[1] == 't') {
			float s, t;
			sscanf_s(line.c_str(), "vt %f %f\n", &s, &t);
			tex[texCount].x = s;	// Need to use X and Y here because XMFLOAT2 doesnt alias its members like GLM
			tex[texCount].y = t;
			++texCount;
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
				tris[triCount][0].x = v1;
				tris[triCount][0].y = t1;
				tris[triCount][0].z = n1;

				tris[triCount][1].x = v2;
				tris[triCount][1].y = t2;
				tris[triCount][1].z = n2;

				tris[triCount][2].x = v3;
				tris[triCount][2].y = t3;
				tris[triCount][2].z = n3;
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


				tris[triCount][0].x = v1;
				tris[triCount][0].y = t1;
				tris[triCount][0].z = n1;

				tris[triCount][1].x = v2;
				tris[triCount][1].y = t2;
				tris[triCount][1].z = n2;

				tris[triCount][2].x = v3;
				tris[triCount][2].y = t3;
				tris[triCount][2].z = n3;
				++triCount;

				tris[triCount][0].x = v3;
				tris[triCount][0].y = t3;
				tris[triCount][0].z = n3;

				tris[triCount][1].x = v4;
				tris[triCount][1].y = t4;
				tris[triCount][1].z = n4;

				tris[triCount][2].x = v1;
				tris[triCount][2].y = t1;
				tris[triCount][2].z = n1;
				++triCount;
			}
		}
	}

	// Three vertices per triangle
	UINT dataLen = triCount * 3;
	returnMesh->mDataLen = dataLen;

	UINT idx = 0;
	returnMesh->mVertexData = new Vertex[dataLen];

	for (UINT i = 0; i < dataLen; ++i) {
		returnMesh->mVertexData[i] = {};
	}

	for (UINT i = 0; i < triCount; ++i) {
		DirectX::XMINT3* tri = tris[i];

		int v1 = tri[0].x;
		int t1 = tri[0].y;
		int n1 = tri[0].z;

		int v2 = tri[1].x;
		int t2 = tri[1].y;
		int n2 = tri[1].z;

		int v3 = tri[2].x;
		int t3 = tri[2].y;
		int n3 = tri[2].z;


		//v1
		Vertex vert1;
		vert1.position = verts[v1 - 1];
		vert1.normal = norms[n1 - 1];
		vert1.texCoord = tex[t1 - 1];

		returnMesh->mVertexData[idx++] = vert1;

		//v2
		Vertex vert2;
		vert2.position = verts[v2 - 1];
		vert2.normal = norms[n2 - 1];
		vert2.texCoord = tex[t2 - 1];

		returnMesh->mVertexData[idx++] = vert2;

		//v3
		Vertex vert3;
		vert3.position = verts[v3 - 1];
		vert3.normal = norms[n3 - 1];
		vert3.texCoord = tex[t3 - 1];

		returnMesh->mVertexData[idx++] = vert3;

	}

	delete[] verts;
	delete[] norms;
	delete[] tex;
	delete[] tris;


	returnMesh->mVertexCount = triCount * 3;

	file.close();

	returnMesh->mVertexBuffer = Dx12Device::CreateBuffer(returnMesh->mVertexData, sizeof(Vertex) * returnMesh->mVertexCount);

	returnMesh->mVertexBufferView = {};
	returnMesh->mVertexBufferView.BufferLocation = returnMesh->mVertexBuffer->mResource->GetGPUVirtualAddress();
	returnMesh->mVertexBufferView.SizeInBytes = returnMesh->mVertexCount * sizeof(Vertex);
	returnMesh->mVertexBufferView.StrideInBytes = sizeof(Vertex);

	return std::move(returnMesh);
}

std::unique_ptr<Mesh> Mesh::CreateMesh(Vertex* vertexData, UINT vertexCount)
{
	std::unique_ptr<Mesh> returnMesh = std::make_unique<Mesh>();

	// Do a deep copy in case the mesh ever needs to be manipulated in flight
	//returnMesh->mVertexData = new Vertex[sizeof(Vertex) * vertexCount];
	returnMesh->mVertexData = new Vertex[vertexCount];
	memcpy(returnMesh->mVertexData, vertexData, sizeof(Vertex) * vertexCount);
	returnMesh->mVertexCount = vertexCount;

	returnMesh->mVertexBuffer = Dx12Device::CreateBuffer(returnMesh->mVertexData, sizeof(Vertex) * returnMesh->mVertexCount);

	returnMesh->mVertexBufferView = {};
	// We need to use the upload buffer if we want to be able to modify the mesh on the fly
	//returnMesh->mVertexBufferView.BufferLocation = returnMesh->mVertexBuffer->mResource->GetGPUVirtualAddress();
	returnMesh->mVertexBufferView.BufferLocation = returnMesh->mVertexBuffer->mUploadBuffer->GetGPUVirtualAddress();
	returnMesh->mVertexBufferView.SizeInBytes = returnMesh->mVertexCount * sizeof(Vertex);
	returnMesh->mVertexBufferView.StrideInBytes = sizeof(Vertex);

	return std::move(returnMesh);
}

// Used to draw debug view of AABB
std::unique_ptr<Mesh> Mesh::CreateFromAABB(Physics::Rectangle aABB)
{
	std::unique_ptr<Mesh> returnMesh = std::make_unique<Mesh>();

	// 24 verts to define the edges of a cube
	int vertCount = 24;
	returnMesh->mVertexData = new Vertex[vertCount];
	memset(returnMesh->mVertexData, 0x00, sizeof(Vertex) * vertCount);
	returnMesh->mVertexCount = vertCount;

	float xMin = aABB.mMin.x;
	float yMin = aABB.mMin.y;
	float zMin = aABB.mMin.z;
	float xMax = aABB.mMax.x;
	float yMax = aABB.mMax.y;
	float zMax = aABB.mMax.z;

	int idx = 0;
	// Trace around the bottom of the mesh
	returnMesh->mVertexData[idx++] = { {xMin, yMin, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMin, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMin, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMax, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMax, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMax, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMax, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMin, zMin}, {0, 0, 0}, {0,0} };
	// Trace around the top of the mesh
	returnMesh->mVertexData[idx++] = { {xMax, yMax, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMax, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMax, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMin, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMin, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMin, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMin, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMax, zMax}, {0, 0, 0}, {0,0} };
	// Add the sides
	returnMesh->mVertexData[idx++] = { {xMax, yMax, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMax, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMax, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMax, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMin, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMin, yMin, zMin}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMin, zMax}, {0, 0, 0}, {0,0} };
	returnMesh->mVertexData[idx++] = { {xMax, yMin, zMin}, {0, 0, 0}, {0,0} };


	returnMesh->mVertexBuffer = Dx12Device::CreateBuffer(returnMesh->mVertexData, sizeof(Vertex) * returnMesh->mVertexCount);
	returnMesh->mVertexBufferView = {};
	// We need to use the upload buffer if we want to be able to modify the mesh on the fly
	//returnMesh->mVertexBufferView.BufferLocation = returnMesh->mVertexBuffer->mResource->GetGPUVirtualAddress();
	returnMesh->mVertexBufferView.BufferLocation = returnMesh->mVertexBuffer->mUploadBuffer->GetGPUVirtualAddress();
	returnMesh->mVertexBufferView.SizeInBytes = returnMesh->mVertexCount * sizeof(Vertex);
	returnMesh->mVertexBufferView.StrideInBytes = sizeof(Vertex);


	return std::move(returnMesh);
}

void Mesh::UpdateBuffer()
{
	mVertexBuffer->UpdateBuffer(0, mVertexCount * sizeof(Vertex), mVertexData);
}

Mesh::~Mesh()
{
	if (mVertexData != nullptr)
	{
		// This is throwing invalid address errors, something wrong
		// with the way the heap is allocated vs deleted I think?
		//delete mVertexData;
		//mVertexData = nullptr;
	}
}