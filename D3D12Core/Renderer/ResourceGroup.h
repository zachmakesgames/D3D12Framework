#pragma once
#include "D3D12Common.h"
#include <unordered_map>
#include "Mesh.h"
#include "RenderObject.h"
#include "Texture.h"

// A self contained way to keep track of resources needed to render a scene. This class
// also allows us to pass these resources between the app and whatever graphics passes
// will be created.
class ResourceGroup
{
public:
	// These will likely stay static for the lifetime of the app
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> mRootSignatures;
	std::unordered_map<std::string, std::unique_ptr<Mesh>> mGeometry;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<IDxcBlob>> mShaders;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> mPSOs;
	std::unordered_map<std::string, Texture> mTextures;

	// These will change with the scene
	std::unordered_map<std::string, std::unique_ptr<RenderObject>> mObjects;
};