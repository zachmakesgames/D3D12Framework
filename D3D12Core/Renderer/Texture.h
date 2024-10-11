#pragma once

#include "D3D12Common.h"

class __declspec(dllexport) Texture
{
public:
	Texture() {}
	Texture(std::string name, std::wstring fileName) : mName(name), mFileName(fileName) {}

	std::string mName = "";
	std::wstring mFileName = L"";
	Microsoft::WRL::ComPtr<ID3D12Resource> mResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadHeap = nullptr;

	ResourceViewHandle mSrvHandle;
};
