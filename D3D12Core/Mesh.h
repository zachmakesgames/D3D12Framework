#pragma once
#include "D3D12Common.h"
#include "Vertex.h"

class __declspec(dllexport) Mesh
{
public:
	static Mesh LoadMeshFromObj(std::string file_name);
	~Mesh();

	Vertex* mVertexData = nullptr;
	uint64_t mVertexCount = 0;
	uint64_t mDataLen = 0;
};

