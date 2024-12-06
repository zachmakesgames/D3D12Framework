#pragma once

#include "Vertex.h"
#include "Dx12Device.h"
#include <dxcapi.h>
#include <array>

class __declspec(dllexport) D3dUtils
{
public:


    // Examples of using the newer IDxcCompiler3 option here:
    // https://microsoft.github.io/hlsl-specs/proposals/0016-c-interface-compiler-library.html

    inline static Microsoft::WRL::ComPtr<IDxcBlob> Dxc3CompileShader(
        const std::wstring& fileName,
        const DxcDefine* defines,
        const std::wstring& entryPoint,
        const std::wstring& target)
    {

        Microsoft::WRL::ComPtr<IDxcLibrary> library;
        HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));

        Microsoft::WRL::ComPtr<IDxcUtils> utils;
        hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        //if(FAILED(hr)) Handle error...

        Microsoft::WRL::ComPtr<IDxcCompiler3> compiler;
        hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        //if(FAILED(hr)) Handle error...

        uint32_t codePage = CP_UTF8;
        Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
        hr = library->CreateBlobFromFile(fileName.c_str(), &codePage, &sourceBlob);
        //if(FAILED(hr)) Handle file loading error...


        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
        sourceBuffer.Size = sourceBlob->GetBufferSize();
        //sourceBuffer.Encoding = 0;
        sourceBuffer.Encoding = CP_UTF8;

        Microsoft::WRL::ComPtr<IDxcIncludeHandler> iHandler;
        utils->CreateDefaultIncludeHandler(&iHandler);


        Microsoft::WRL::ComPtr<IDxcOperationResult> result;
        Microsoft::WRL::ComPtr<IDxcResult> compileResult;

        std::vector<const wchar_t*> args;
        args.push_back(L"-E");
        args.push_back(entryPoint.c_str());
        args.push_back(L"-T");
        args.push_back(target.c_str());
        args.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
        args.push_back(L"-Zi");
        args.push_back(L"-Qembed_debug");

        hr = compiler->Compile(
            &sourceBuffer,
            args.data(),
            static_cast<uint32_t>(args.size()),
            nullptr,
            IID_PPV_ARGS(&result));

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
        else
        {
            if (result)
            {
                Microsoft::WRL::ComPtr<IDxcBlobEncoding> errorsBlob;
                hr = result->GetErrorBuffer(&errorsBlob);
                if (SUCCEEDED(hr) && errorsBlob)
                {
                    OutputDebugStringA("Dxc output:");
                    OutputDebugStringA((const char*)errorsBlob->GetBufferPointer());
                    OutputDebugStringA("\r\n");
                    // wprintf(L"Compilation failed with errors:\n%hs\n",
                    //     (const char*)errorsBlob->GetBufferPointer());
                }
            }
        }
        Microsoft::WRL::ComPtr<IDxcBlob> code;
        result->GetResult(&code);

        return code;
    }

    // Massive shoutout to Adam Sawicki for making a lot of this possible in his blog:
    // https://asawicki.info/news_1719_two_shader_compilers_of_direct3d_12

	inline static Microsoft::WRL::ComPtr<IDxcBlob> DxcCompileShader(
		const std::wstring& fileName,
		const DxcDefine* defines,
		const std::wstring& entryPoint,
		const std::wstring& target)
	{

        TCHAR buffer[MAX_PATH] = { 0 };
        GetModuleFileName(NULL, buffer, MAX_PATH);
        std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
        std::wstring pathstr = std::wstring(buffer).substr(0, pos);

        std::wstring pathmsg = L"Current path: " + pathstr + L"\r\n";
        OutputDebugString(pathmsg.c_str());

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

        const wchar_t *args[] = {
            L"/WX",
            L"/Zi",
            L"-Qembed_debug",
        };

        hr = compiler->Compile(
            sourceBlob.Get(), // pSource
            fileName.c_str(), // pSourceName
            entryPoint.c_str(), // pEntryPoint
            target.c_str(), // pTargetProfile
            //NULL, 0, // pArguments, argCount
            args, 3,
            //defines, sizeof(defines) / sizeof(DxcDefine), // pDefines, defineCount
            NULL, 0,
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
        else
        {
            if (result)
            {
                Microsoft::WRL::ComPtr<IDxcBlobEncoding> errorsBlob;
                hr = result->GetErrorBuffer(&errorsBlob);
                if (SUCCEEDED(hr) && errorsBlob)
                {
                    OutputDebugStringA("Dxc output:");
                    OutputDebugStringA((const char*)errorsBlob->GetBufferPointer());
                    OutputDebugStringA("\r\n");
                   // wprintf(L"Compilation failed with errors:\n%hs\n",
                   //     (const char*)errorsBlob->GetBufferPointer());
                }
            }
        }
        Microsoft::WRL::ComPtr<IDxcBlob> code;
        result->GetResult(&code);

        return code;
	}

    // Common static samplers that could be used for many applications.
    // Couresty of Frank Luna 
    inline static const std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers()
    {
        const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
            0, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

        const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
            1, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

        const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
            2, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

        const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
            3, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

        const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
            4, // shaderRegister
            D3D12_FILTER_ANISOTROPIC, // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
            0.0f,                             // mipLODBias
            8);                               // maxAnisotropy

        const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
            5, // shaderRegister
            D3D12_FILTER_ANISOTROPIC, // filter
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
            0.0f,                              // mipLODBias
            8);                                // maxAnisotropy

        const CD3DX12_STATIC_SAMPLER_DESC shadow(
            6, // shaderRegister
            D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
            0.0f,                               // mipLODBias
            16,                                 // maxAnisotropy
            D3D12_COMPARISON_FUNC_LESS_EQUAL,
            D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

        return {
            pointWrap, pointClamp,
            linearWrap, linearClamp,
            anisotropicWrap, anisotropicClamp,
            shadow
        };
    }


    inline static std::string MatrixToString(DirectX::XMFLOAT4X4 matrix)
    {
        std::string matrixStr = "";
        matrixStr += std::to_string(matrix._11) + ", " + std::to_string(matrix._12) + ", " + std::to_string(matrix._13) + ", " + std::to_string(matrix._14) + "\r\n";
        matrixStr += std::to_string(matrix._21) + ", " + std::to_string(matrix._22) + ", " + std::to_string(matrix._23) + ", " + std::to_string(matrix._24) + "\r\n";
        matrixStr += std::to_string(matrix._31) + ", " + std::to_string(matrix._32) + ", " + std::to_string(matrix._33) + ", " + std::to_string(matrix._34) + "\r\n";
        matrixStr += std::to_string(matrix._41) + ", " + std::to_string(matrix._42) + ", " + std::to_string(matrix._43) + ", " + std::to_string(matrix._44);

        return matrixStr;
    }

    inline static std::string VectorToString(DirectX::XMFLOAT4 vector)
    {
        std::string vectorStr = "";
        vectorStr += std::to_string(vector.x) + ", " + std::to_string(vector.y) + ", " + std::to_string(vector.z) + ", " + std::to_string(vector.w) + "\r\n";

        return vectorStr;
    }

    inline static std::string VectorToString(DirectX::XMFLOAT3 vector)
    {
        std::string vectorStr = "";
        vectorStr += std::to_string(vector.x) + ", " + std::to_string(vector.y) + ", " + std::to_string(vector.z) + "\r\n";

        return vectorStr;
    }
};


/// SECTION
/// Fast Hash

// Original Source: https://github.com/ztanml/fast-hash/blob/master/fasthash.c

/* The MIT License

   Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/


// Compression function for Merkle-Damgard construction.
inline static constexpr uint64_t mixHash(uint64_t h)
{
    (h) ^= (h) >> 23;
    (h) *= 0x2127599bf4325c37ULL;
    (h) ^= (h) >> 47;

    return h;
}

inline static constexpr uint64_t fasthash64(const void* buf, size_t len, uint64_t seed)
{
    const uint64_t    m = 0x880355f21e6d1965ULL;
    const uint64_t* pos = (const uint64_t*)buf;
    const uint64_t* end = pos + (len / 8);
    const unsigned char* pos2 = nullptr;
    uint64_t h = seed ^ (len * m);
    uint64_t v = 0;

    while (pos != end) {
        v = *pos++;
        h ^= mixHash(v);
        h *= m;
    }

    pos2 = (const unsigned char*)pos;
    v = 0;

    switch (len & 7) {
    case 7: v ^= (uint64_t)pos2[6] << 48;
    case 6: v ^= (uint64_t)pos2[5] << 40;
    case 5: v ^= (uint64_t)pos2[4] << 32;
    case 4: v ^= (uint64_t)pos2[3] << 24;
    case 3: v ^= (uint64_t)pos2[2] << 16;
    case 2: v ^= (uint64_t)pos2[1] << 8;
    case 1: v ^= (uint64_t)pos2[0];
        h ^= mixHash(v);
        h *= m;
    }

    return mixHash(h);
}

/// END SECTION
/// Fast Hash

inline static constexpr uint64_t HashLiteralString(const char* str)
{
    return fasthash64(str, strlen(str), 0);
}

inline constexpr uint64_t operator""_h(const char* str, size_t)
{
    return HashLiteralString(str);
}