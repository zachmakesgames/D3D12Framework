#pragma once

#include <windows.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <exception>
#include <string>
#include <memory>
#include <array>
#include "d3dx12.h"
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <dxcapi.h>
#include <pix3.h>

#pragma comment(lib, "WinPixEventRuntime.lib")


struct ResourceViewHandle
{
    UINT mOffset = 0;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuHandle = {};
    CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuHandle = {};
};

/// 
/// Section: Borrowed code from Frank Luna
/// 
class DxException
{
public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
};

inline std::wstring AnsiToWString(const std::string& str)
{
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
}



#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

/// 
/// End Section: Borrowed code from Frank Luna
/// 
