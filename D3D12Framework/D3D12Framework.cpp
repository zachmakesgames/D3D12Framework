// D3D12Framework.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "D3D12Framework.h"
#include "Dx12Device.h"
#include "GraphicsPass.h"
#include "RenderObject.h"

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

    Dx12Device::Create();
    Dx12Device::InitSwapchain(hWnd, 300, 300);

    auto cmdListAlloc = Dx12Device::GetCurrentFrameAllocator();
    auto cmdList = Dx12Device::GetCommandList();
    auto cmdQueue = Dx12Device::GetCommandQueue();

    // Dont reset the allocator if its never been associated to a command list
    //cmdListAlloc->Reset();
    HRESULT hr = cmdList->Reset(cmdListAlloc.Get(), nullptr);
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to reset command list");
    }

    D3D12_VIEWPORT viewPort = {};
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = 300;
    viewPort.Height = 300;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;

    D3D12_RECT rect = { 0, 0, 300, 300 };

    cmdList->RSSetViewports(1, &viewPort);
    cmdList->RSSetScissorRects(1, &rect);

    D3D12_CPU_DESCRIPTOR_HANDLE views[] = { Dx12Device::GetCurrentBackBufferView() };
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = Dx12Device::GetDepthStencilView();
    cmdList->OMSetRenderTargets(1, views, true, &dsv);

    float color[] = { 0.2, 0.2, 0.2, 1 };
    cmdList->ClearRenderTargetView(Dx12Device::GetCurrentBackBufferView(), color, 1, &rect);

    cmdList->Close();

    ID3D12CommandList* lists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(lists), lists);

    Dx12Device::Present();

    

    RenderObject carObject;
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

    OutputDebugStringA(graphString.c_str());




    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_D3D12FRAMEWORK));

    MSG msg;

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

   *hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

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
