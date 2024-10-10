#include "Mesh.h"

#include <string>
#include <iostream>
#include <fstream>

Mesh Mesh::LoadMeshFromObj(std::string file_name)
{
	Mesh returnMesh;
	std::ifstream file;
	//this->modelName = objFileName;
	file.open(file_name, std::fstream::in);
	if (!file) {
		OutputDebugStringA("ERROR: Tried to load a mesh from file, but file was not found!");
		return returnMesh;
	}
	int lines = 0;
	std::string line = "";
	while (std::getline(file, line)) {
		++lines;
	}
	file.clear();
	file.seekg(0);


	uint64_t vertCount = 0;
	uint64_t normCount = 0;
	uint64_t texCount = 0;
	uint64_t triCount = 0;

	uint64_t maxTris = lines * 3;



	//Initialize the arrays


	DirectX::XMFLOAT3* verts = new DirectX::XMFLOAT3[lines];
	DirectX::XMFLOAT3* norms = new DirectX::XMFLOAT3[lines];
	DirectX::XMFLOAT2* tex = new DirectX::XMFLOAT2[lines];

	DirectX::XMFLOAT3** tris = new DirectX::XMFLOAT3 * [lines];
	for (int i = 0; i < lines; ++i) {
		//Special note: Using an ivec3 istead of a vec3 to avoid roundoff errors and compiler warnings
		tris[i] = new DirectX::XMFLOAT3[3];//using glm::vec3 creatively here. 
		//OBJ file recoreds faces as: f v1/t1/n1 v2/t2/n2 v3/t3/n3
		//so tris[n][0].x = v1 .y = t1 .z = n1
		//tris[n][1].x = v2 .y = t2 .z = n2
		//tris[n][2].x = v3 .y = t3 .z = n3
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

	int flop = 0;


	//fprintf(stderr, "Triangle counts in mesh %s: %i\n",this->modelName.c_str(),  triCount);
	//int dataLen = triCount * 3 + triCount * 3 + triCount *2;
	//int dataLen = triCount * 8 * 3;


	// Three vertices per triangle
	int dataLen = triCount * 3;
	//this->dataSize = dataLen;
	returnMesh.mDataLen = dataLen;

	int idx = 0;
	//GLfloat* data = new GLfloat[dataLen];
	//this->data = new vertex[dataLen];
	returnMesh.mVertexData = new Vertex[dataLen];

	for (int i = 0; i < dataLen; ++i) {
		//data[i] = {};
		returnMesh.mVertexData[i] = {};
	}

	for (int i = 0; i < triCount; ++i) {
		DirectX::XMFLOAT3* tri = tris[i];
		//glm::ivec3* tri = tris[i];

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
		//vert1.color = glm::vec3(1.0, 1.0, 1.0);

		returnMesh.mVertexData[idx++] = vert1;
		//data[idx++] = vert1;

		//v2
		Vertex vert2;
		vert2.position = verts[v2 - 1];
		vert2.normal = norms[n2 - 1];
		vert2.texCoord = tex[t2 - 1];
		//vert2.color = glm::vec3(1.0, 1.0, 1.0);

		//data[idx++] = vert2;
		returnMesh.mVertexData[idx++] = vert2;

		//v3
		Vertex vert3;
		vert3.position = verts[v3 - 1];
		vert3.normal = norms[n3 - 1];
		vert3.texCoord = tex[t3 - 1];
		//vert3.color = glm::vec3(1.0, 1.0, 1.0);

		//c_Triangle phys_triangle(vert1.position, vert2.position, vert3.position);
		//m_triangles.push_back(phys_triangle);

		//data[idx++] = vert3;
		returnMesh.mVertexData[idx++] = vert3;

	}

	delete[] verts;
	delete[] norms;
	delete[] tex;
	delete[] tris;

	//this->verts = triCount * 3;
	returnMesh.mVertexCount = triCount * 3;

	/*bounding_volume_center = glm::vec3(0, 0, 0);

	for (uint64_t i = 0; i < this->verts; ++i)
	{
		bounding_volume_center += data[i].position;
	}

	bounding_volume_center /= this->verts;

	float furthest_len_sqr = 0.0f;

	for (uint64_t i = 0; i < this->verts; ++i)
	{
		glm::vec3 vert = data[i].position;

		glm::vec3 vec = vert - bounding_volume_center;

		float sqr_dist = glm::dot(vec, vec);

		if (sqr_dist > furthest_len_sqr)
		{
			furthest_len_sqr = sqr_dist;
		}
	}

	bounding_volume_radius = sqrt(furthest_len_sqr);
	*/

	file.close();

	return returnMesh;
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