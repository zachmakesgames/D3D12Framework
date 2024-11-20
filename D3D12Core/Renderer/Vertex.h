#pragma once
#include "D3D12Common.h"

static const std::array<D3D12_INPUT_ELEMENT_DESC, 3> sVertexInputLayout 
{{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
}};

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoord;
};

struct SimpleVertex
{
	DirectX::XMFLOAT3 vert;
};


static Vertex sFullScreenTriangle[] =
{
	{ {1.f, 3.f, 0.01f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {1.f, -1.f, 0.01f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-3.f, -1.f, 0.01f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },

	// Wrong winding order
	/*{ {-3, -1, 0.01}, {0, 0, 0}, {0, 0} },
	{ {1, -1, 0.01}, {0, 0, 0}, {0, 0} },
	{ {1, 3, 0.01}, {0, 0, 0}, {0, 0} },*/
};

static Vertex sLine[] =
{
	{ {10.f,   10.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-10.f, -10.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
};

// Line list to draw a 1x1x1 cube
static Vertex sCube[] =
{
	{ {0.5f,   0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f,  0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,   0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,   0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f,  0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,   0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f,  0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f,  0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,  -0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f, -0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,  -0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,  -0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f, -0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,  -0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f, -0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f, -0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f,  0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f, -0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,   0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,  -0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,   0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {0.5f,  -0.5f, -0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f,  0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
	{ {-0.5f, -0.5f,  0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f} },
};