#pragma once
#include "D3D12Common.h"
#include "Dx12Device.h"
#include "Vertex.h"
#include "Buffer.h"
#include "../Physics/PhysicsPrimitives.h"

class __declspec(dllexport) Mesh
{
public:
	inline Mesh()
	{}

	static std::unique_ptr<Mesh> LoadMeshFromObj(std::string file_name);
	static std::unique_ptr<Mesh> LoadMeshFromObj2(std::string file_name);
	static std::unique_ptr<Mesh> CreateMesh(Vertex* vertexData, UINT vertexCount);
	static std::unique_ptr<Mesh> CreateFromAABB(Physics::Rectangle aABB);
	
	void UpdateBuffer();
	~Mesh();

	// Warning, DO NOT resize this array. The data can
	// be modified but the resource buffer does not get
	// resized and the view does not change. Only change
	// existing vertices!
	Vertex* mVertexData = nullptr;
	UINT mVertexCount = 0;
	UINT mDataLen = 0;

	std::unique_ptr<Buffer> mVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView = {};
};

