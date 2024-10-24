#pragma once
#include "D3D12Common.h"
#include "Dx12Device.h"
#include "Vertex.h"
#include "Buffer.h"

class __declspec(dllexport) Mesh
{
public:
	inline Mesh()
	{}

	static std::unique_ptr<Mesh> LoadMeshFromObj(std::string file_name);
	static std::unique_ptr<Mesh> CreateMesh(Vertex* vertexData, UINT vertexCount);
	~Mesh();

	Vertex* mVertexData = nullptr;
	uint64_t mVertexCount = 0;
	uint64_t mDataLen = 0;

	std::unique_ptr<Buffer> mVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;

};

