#pragma once

#include "Vertex.h"
#include "Dx12Device.h"
#include <dxcapi.h>

class __declspec(dllexport) D3dUtils
{

    // Massive shoutout to Adam Sawicki for making a lot of this possible in his blog:
    // https://asawicki.info/news_1719_two_shader_compilers_of_direct3d_12

	inline static Microsoft::WRL::ComPtr<IDxcBlob> DxcCompileShader(
		const std::wstring& fileName,
		const D3D_SHADER_MACRO* defines,
		const std::string& entryPoint,
		const std::string& target)
	{
        Microsoft::WRL::ComPtr<IDxcLibrary> library;
        HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

        Microsoft::WRL::ComPtr<IDxcUtils> utils;
        hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        //if(FAILED(hr)) Handle error...

        Microsoft::WRL::ComPtr<IDxcCompiler> compiler;
        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        //if(FAILED(hr)) Handle error...

        uint32_t codePage = CP_UTF8;
        Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
        hr = library->CreateBlobFromFile(fileName.c_str(), &codePage, &sourceBlob);
        //if(FAILED(hr)) Handle file loading error...

        Microsoft::WRL::ComPtr<IDxcIncludeHandler> iHandler;
        utils->CreateDefaultIncludeHandler(&iHandler);

        Microsoft::WRL::ComPtr<IDxcOperationResult> result;
        hr = compiler->Compile(
            sourceBlob.Get(), // pSource
            fileName.c_str(), // pSourceName
            entryPoint.c_str(), // pEntryPoint
            target.c_str(), // pTargetProfile
            NULL, 0, // pArguments, argCount
            defines, sizeof(defines) / sizeof(DxcDefine), // pDefines, defineCount
            iHandler.Get(), // pIncludeHandler
            &result); // ppResult
        if (SUCCEEDED(hr))
            result->GetStatus(&hr);
        if (FAILED(hr))
        {
            if (result)
            {
                Microsoft::WRL::ComPtr<IDxcBlobEncoding> errorsBlob;
                hr = result->GetErrorBuffer(&errorsBlob);
                if (SUCCEEDED(hr) && errorsBlob)
                {
                    OutputDebugStringA("Failed to compile shader:");
                    OutputDebugStringA((const char*)errorsBlob->GetBufferPointer());
                    wprintf(L"Compilation failed with errors:\n%hs\n",
                        (const char*)errorsBlob->GetBufferPointer());
                }
            }
            // Handle compilation error...
        }
        Microsoft::WRL::ComPtr<IDxcBlob> code;
        result->GetResult(&code);

        return code;
	}
};