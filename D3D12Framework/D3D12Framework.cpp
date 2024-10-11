// D3D12Framework.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "D3D12Framework.h"
#include "Dx12Device.h"
#include "D3D12Common.h"
#include "GraphicsPass.h"
#include "RenderObject.h"
#include "Mesh.h"
#include "Utils.h"
#include "Vertex.h"
#include "Texture.h"
#include "ConstantBufferStructs.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND* hWnd);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_D3D12FRAMEWORK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);


    HWND hWnd;
    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow, &hWnd))
    {
        return FALSE;
    }

    HRESULT hr;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D3D12FRAMEWORK));

    MSG msg;

    Dx12Device::Create();
    Dx12Device::InitSwapchain(hWnd, 600, 600);

    auto cmdListAlloc = Dx12Device::GetCurrentFrameAllocator();
    auto cmdList = Dx12Device::GetCommandList();
    auto cmdQueue = Dx12Device::GetCommandQueue();

    //Dx12Device::InitGBuffer(600, 600);


    // Dont reset the allocator if its never been associated to a command list
    //cmdListAlloc->Reset();
    hr = cmdList->Reset(cmdListAlloc.Get(), nullptr);
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to reset command list");
    }


    CD3DX12_DESCRIPTOR_RANGE textureTable0;
    textureTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    CD3DX12_ROOT_PARAMETER slotRootParam[3];
    slotRootParam[0].InitAsConstantBufferView(0);
    slotRootParam[1].InitAsConstantBufferView(1);
    slotRootParam[2].InitAsDescriptorTable(1, &textureTable0, D3D12_SHADER_VISIBILITY_PIXEL);

    auto samplers = D3dUtils::GetStaticSamplers();

    CD3DX12_ROOT_SIGNATURE_DESC mainRootSig(3, slotRootParam,
        (UINT)samplers.size(), samplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);


    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSig = Dx12Device::CreateRootSignature(&mainRootSig);

    UINT vertexSize = sizeof(Vertex);

    
    std::unique_ptr<Mesh> boxMesh = Mesh::LoadMeshFromObj("../../../Resources/Models/box.obj");

    
    auto vertexShader = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/test.hlsl", nullptr, L"vsMain", L"vs_6_6");
    auto pixelShader = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/test.hlsl", nullptr, L"psMain", L"ps_6_6");
   
    Texture testTexture("TestPattern", L"../../../Resources/Textures/TestPattern.dds");
    hr = Dx12Device::LoadTextureFromDDSFile(&testTexture);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to load DDS texture");
    }


    DirectX::XMMATRIX ident = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT4X4 identF;

    DirectX::XMStoreFloat4x4(&identF, ident);
    cbWorld worldBuffer = {};
    DirectX::XMFLOAT4X4 projection = identF;
    DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, 300.f / 300.f, 1.0f, 1000.0f);

    DirectX::XMStoreFloat4x4(&projection, XMMatrixTranspose(projMat));

    worldBuffer.mProjMat = projection;
    worldBuffer.mViewMat = identF;


    // Push the box out in front of the camera (Z axis) and slightly to the right (X axis)
    // so we can see the projection is working right.
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0, 0, 10);
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationX(0.3 * DirectX::XM_PI);
    DirectX::XMMATRIX rotation2 = DirectX::XMMatrixRotationY(0.3 * DirectX::XM_PI);

    DirectX::XMMATRIX rot = DirectX::XMMatrixMultiply(rotation, rotation2);

    DirectX::XMMATRIX transform = DirectX::XMMatrixMultiply(rot, translation);
    DirectX::XMFLOAT4X4 translationFloat;
    DirectX::XMStoreFloat4x4(&translationFloat, XMMatrixTranspose(transform));
    cbObject objectBuffer = {};
    objectBuffer.mWorldTransform = translationFloat;

    auto worldCBuffer = Dx12Device::CreateBuffer(&worldBuffer, sizeof(cbWorld));
    auto objectCBuffer = Dx12Device::CreateBuffer(&objectBuffer, sizeof(cbObject));

    D3D12_CONSTANT_BUFFER_VIEW_DESC worldCBufferViewDesc = {};
    worldCBufferViewDesc.BufferLocation = worldCBuffer->mResource->GetGPUVirtualAddress();
    worldCBufferViewDesc.SizeInBytes = 256;

    D3D12_CONSTANT_BUFFER_VIEW_DESC objectCBufferViewDesc = {};
    objectCBufferViewDesc.BufferLocation = objectCBuffer->mResource->GetGPUVirtualAddress();
    objectCBufferViewDesc.SizeInBytes = 256;

    auto worldCBV = Dx12Device::CreateConstantBufferView(&worldCBufferViewDesc);
    auto objectCBV = Dx12Device::CreateConstantBufferView(&objectCBufferViewDesc);
    

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
    psoDesc.pRootSignature = rootSig.Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize()};
    psoDesc.PS = { reinterpret_cast<BYTE*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = Dx12Device::GetBackBufferFormat();
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = Dx12Device::GetDepthStencilFormat();

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso = Dx12Device::CreatePSO(&psoDesc);


    cmdList->Close();

    ID3D12CommandList* lists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(lists), lists);
    Dx12Device::FlushQueue();

    // Draw a few frames to have enough time to capture in Pix for debugging
    for (int i = 0; i < 1000; ++i)
    {
        std::wstring framenum = L"Frame number: " + std::to_wstring(i) + L"\r\n";
        OutputDebugString(framenum.c_str());
        cmdListAlloc = Dx12Device::GetCurrentFrameAllocator();
        cmdListAlloc->Reset();

        cmdList->Reset(cmdListAlloc.Get(), pso.Get());

        D3D12_VIEWPORT viewPort = {};
        viewPort.TopLeftX = 0;
        viewPort.TopLeftY = 0;
        viewPort.Width = 600;
        viewPort.Height = 600;
        viewPort.MinDepth = 0.0f;
        viewPort.MaxDepth = 1.0f;

        D3D12_RECT rect = { 0, 0, 600, 600 };

        cmdList->RSSetViewports(1, &viewPort);
        cmdList->RSSetScissorRects(1, &rect);

        D3D12_CPU_DESCRIPTOR_HANDLE views[] = { Dx12Device::GetCurrentBackBufferView() };
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = Dx12Device::GetDepthStencilView();
        cmdList->OMSetRenderTargets(1, views, true, &dsv);

        // Transition to render target state
        CD3DX12_RESOURCE_BARRIER presentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(Dx12Device::GetCurrentBackBuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmdList->ResourceBarrier(1, &presentToRenderTarget);


        float color[] = { 0.2, 0.2, 0.5, 1 };
        cmdList->ClearRenderTargetView(Dx12Device::GetCurrentBackBufferView(), color, 1, &rect);
        cmdList->ClearDepthStencilView(Dx12Device::GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);


        // The last heap is the CbvSrvUav heap, only bind it for now
        auto heaps = Dx12Device::GetDescriptorHeaps();
        cmdList->SetDescriptorHeaps(1, &heaps[2]);

        cmdList->SetGraphicsRootSignature(rootSig.Get());


        cmdList->SetGraphicsRootConstantBufferView(0, worldCBuffer->mResource->GetGPUVirtualAddress());
        cmdList->SetGraphicsRootConstantBufferView(1, objectCBuffer->mResource->GetGPUVirtualAddress());    
        cmdList->SetGraphicsRootDescriptorTable(2, testTexture.mSrvHandle.mGpuHandle);

        cmdList->SetPipelineState(pso.Get());


        cmdList->IASetVertexBuffers(0, 1, &boxMesh->mVertexBufferView);
        cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


        //should have 36 vertices for a cube
        UINT count = boxMesh->mVertexCount;
        cmdList->DrawInstanced(count, 1, 0, 0);



        // Transition to present state
        CD3DX12_RESOURCE_BARRIER renderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(Dx12Device::GetCurrentBackBuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        cmdList->ResourceBarrier(1, &renderTargetToPresent);


        cmdList->Close();

        ID3D12CommandList* lists[] = { cmdList.Get() };
        cmdQueue->ExecuteCommandLists(_countof(lists), lists);


        Dx12Device::Present();
        Dx12Device::FlushQueue();

        // Need to handle messages in order to capture frames in pix,
        // oops lol
        if (GetMessage(&msg, nullptr, 0, 0))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

    }
    

    /*RenderObject carObject;
    RenderObject trackObject;
    RenderObject treeObject;

    RenderObject fullScreenDaw;


    PassGraph testGraph;
    GraphicsPass preDepth("PreDepth");
    preDepth.mRenderObjectRefs.push_back(&carObject);
    preDepth.mRenderObjectRefs.push_back(&trackObject);
    preDepth.mRenderObjectRefs.push_back(&treeObject);

    GraphicsPass GBuffer("GBuffer");
    GBuffer.mRenderObjectRefs.push_back(&carObject);
    GBuffer.mRenderObjectRefs.push_back(&trackObject);
    GBuffer.mRenderObjectRefs.push_back(&treeObject);


    GraphicsPass Lighting("Lighting");
    Lighting.mRenderObjectRefs.push_back(&fullScreenDaw);

    GraphicsPass PostProcess("PostProcess");
    PostProcess.mRenderObjectRefs.push_back(&fullScreenDaw);

    testGraph.AddPass(&preDepth);
    testGraph.AddPass(&GBuffer);
    testGraph.AddPass(&Lighting);
    testGraph.AddPass(&PostProcess);

    std::string graphString = testGraph.Walk();

    OutputDebugStringA(graphString.c_str());*/




    

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_D3D12FRAMEWORK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;// MAKEINTRESOURCEW(IDC_D3D12FRAMEWORK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND* hWnd)
{
   hInst = hInstance; // Store instance handle in our global variable

   RECT r = { 0, 0, 600, 600 };
   AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, true);

   /**hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);*/
   *hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       100, 200, r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(*hWnd, nCmdShow);
   UpdateWindow(*hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
