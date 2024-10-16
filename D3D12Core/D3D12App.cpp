#include "D3D12App.h"

D3D12App::D3D12App(HWND window, int swapchainWidth, int swapchainHeight, bool windowed):
	mWindow(window), mWidth(swapchainWidth), mHeight(swapchainHeight), mWindowed(windowed)
{
}

void D3D12App::Init()
{
	Dx12Device::Create();
	Dx12Device::InitSwapchain(mWindow, mWidth, mHeight, mWindowed);

    // Reset the command list for resource uploads
    auto cmdListAlloc = Dx12Device::GetCurrentFrameAllocator();
    auto cmdList = Dx12Device::GetCommandList();
    auto cmdQueue = Dx12Device::GetCommandQueue();

    HRESULT hr = cmdList->Reset(cmdListAlloc.Get(), nullptr);
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to reset command list");
    }

    // Init all the things, create all the things, do ALL the THINGS!!!!
	CreateRootSigs();
    CreateGeometry();
    CreateShaders();
    CreatePSOs();

    
    mResourceGroup.mObjects["box"] = std::make_unique<RenderObject>("box");
    mResourceGroup.mObjects["box"]->mTextureRef = "TestPattern";
    mResourceGroup.mObjects["box2"] = std::make_unique<RenderObject>("box");
    mResourceGroup.mObjects["box2"]->mTextureRef = "TestPattern";

    // Need to keep a record of the passes, otherwise they get destructed
    mPasses["mainPass"] = new ForwardPass(&mResourceGroup, &mConstants);
    mPasses["mainPass"]->mPassName = "Forward pass";

    mPasses["mainPass"]->RegisterRenderObject(mResourceGroup.mObjects["box"].get());
    mPasses["mainPass"]->RegisterRenderObject(mResourceGroup.mObjects["box2"].get());

    mPasses["deferredPass"] = new GBufferPass(&mResourceGroup, &mConstants);
    mPasses["deferredPass"]->mPassName = "Deferred pass";

    mPasses["deferredPass"]->RegisterRenderObject(mResourceGroup.mObjects["box"].get());
    mPasses["deferredPass"]->RegisterRenderObject(mResourceGroup.mObjects["box2"].get());

    mPassGraph.AddPass(mPasses["mainPass"]);
    mPassGraph.AddPass(mPasses["deferredPass"]);

    DirectX::XMMATRIX ident = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT4X4 identF;

    DirectX::XMStoreFloat4x4(&identF, ident);
    
    DirectX::XMFLOAT4X4 projection = identF;
    DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, 300.f / 300.f, 1.0f, 1000.0f);

    DirectX::XMStoreFloat4x4(&projection, XMMatrixTranspose(projMat));

    mConstants.mWorldConstants.mProjMat = projection;
    mConstants.mWorldConstants.mViewMat = identF;
    
    mConstants.Init();


    // Push the box out in front of the camera (Z axis) and slightly to the right (X axis)
    // so we can see the projection is working right.
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0, 0, 10);
    
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationX(0.3 * DirectX::XM_PI);
    DirectX::XMMATRIX rotation2 = DirectX::XMMatrixRotationY(0.3 * DirectX::XM_PI);

    DirectX::XMMATRIX rot = DirectX::XMMatrixMultiply(rotation, rotation2);

    DirectX::XMMATRIX transform = DirectX::XMMatrixMultiply(rot, translation);
    DirectX::XMFLOAT4X4 translationFloat;
    DirectX::XMStoreFloat4x4(&translationFloat, XMMatrixTranspose(transform));

    mResourceGroup.mObjects["box"]->mConstants.worldTransform = translationFloat;

    DirectX::XMMATRIX translation2 = DirectX::XMMatrixTranslation(-5, 0, 15);
    DirectX::XMMATRIX transform2 = DirectX::XMMatrixMultiply(rot, translation2);
    DirectX::XMFLOAT4X4 translationFloat2;
    DirectX::XMStoreFloat4x4(&translationFloat2, XMMatrixTranspose(transform2));
    mResourceGroup.mObjects["box2"]->mConstants.worldTransform = translationFloat2;


    cmdList->Close();

    ID3D12CommandList* lists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(lists), lists);
    Dx12Device::FlushQueue(true);
}

// TODO: Leave this for only very common root signatures, allow GraphicsPasses to create their own resources
void D3D12App::CreateRootSigs()
{
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

    mResourceGroup.mRootSignatures["mainRootSig"] = Dx12Device::CreateRootSignature(&mainRootSig);


    CD3DX12_DESCRIPTOR_RANGE gBufferSrvTable;
    gBufferSrvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0);

    CD3DX12_ROOT_PARAMETER lightingRootParam[1];
    lightingRootParam[0].InitAsDescriptorTable(1, &gBufferSrvTable, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_ROOT_SIGNATURE_DESC lightingRootSig(1, lightingRootParam,
        (UINT)samplers.size(), samplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    mResourceGroup.mRootSignatures["lightingRootSig"] = Dx12Device::CreateRootSignature(&lightingRootSig);
}

void D3D12App::CreateGeometry()
{
    mResourceGroup.mGeometry["box"] = Mesh::LoadMeshFromObj("../../../Resources/Models/box.obj");

    // whoops, this is duplicated from the init function
    //mResourceGroup.mObjects["boxObj"] = std::make_unique<RenderObject>("box");

    float x, y, w, h;
    x = -1;
    y = -1;
    w = 2;
    h = 2;

    Vertex triangle[] =
    {
        { {x, y, 0.01}, {0, 0, 0}, {0, 0} },
        { {x + w, y, 0.01}, {0, 0, 0}, {0, 0} },
        { {x + w, y + h, 0.01}, {0, 0, 0}, {0, 0} },
    };

    mResourceGroup.mGeometry["triangle"] = Mesh::CreateMesh(triangle, 3);



    // TODO: Why is this here? Where does this go? How does a computer even work?!
    mResourceGroup.mTextures["TestPattern"] = Texture("TestPattern", L"../../../Resources/Textures/TestPattern.dds");
    HRESULT hr = Dx12Device::LoadTextureFromDDSFile(&mResourceGroup.mTextures["TestPattern"]);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to load DDS texture!\r\n");
    }
}

// TODO: Leave this for only very common shaders, allow GraphicsPasses to create their own resources
void D3D12App::CreateShaders()
{
    mResourceGroup.mShaders["simpleVs"] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/test.hlsl", nullptr, L"vsMain", L"vs_6_6");
    mResourceGroup.mShaders["simplePs"] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/test.hlsl", nullptr, L"psMain", L"ps_6_6");

    mResourceGroup.mShaders["deferredVs"] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/deferred.hlsl", nullptr, L"vsMain", L"vs_6_6");
    mResourceGroup.mShaders["deferredPs"] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/deferred.hlsl", nullptr, L"psMain", L"ps_6_6");

    mResourceGroup.mShaders["deferredLightingVs"] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/deferredlighting.hlsl", nullptr, L"vsMain", L"vs_6_6");
    mResourceGroup.mShaders["deferredLightingPs"] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/deferredlighting.hlsl", nullptr, L"psMain", L"ps_6_6");
}


// TODO: Leave this for only very common PSOs, allow GraphicsPasses to create their own resources
void D3D12App::CreatePSOs()
{
    GBuffer* gBuffer = Dx12Device::GetGBuffer();

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
    psoDesc.pRootSignature = mResourceGroup.mRootSignatures["mainRootSig"].Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["simpleVs"]->GetBufferPointer()), mResourceGroup.mShaders["simpleVs"]->GetBufferSize()};
    psoDesc.PS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["simplePs"]->GetBufferPointer()), mResourceGroup.mShaders["simplePs"]->GetBufferSize() };
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

    mResourceGroup.mPSOs["simplePSO"] = Dx12Device::CreatePSO(&psoDesc);

    psoDesc.pRootSignature = mResourceGroup.mRootSignatures["lightingRootSig"].Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["deferredLightingVs"]->GetBufferPointer()), mResourceGroup.mShaders["deferredLightingVs"]->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["deferredLightingPs"]->GetBufferPointer()), mResourceGroup.mShaders["deferredLightingPs"]->GetBufferSize() };

    mResourceGroup.mPSOs["deferredLightingPSO"] = Dx12Device::CreatePSO(&psoDesc);

    psoDesc.pRootSignature = mResourceGroup.mRootSignatures["mainRootSig"].Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["deferredVs"]->GetBufferPointer()), mResourceGroup.mShaders["deferredVs"]->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["deferredPs"]->GetBufferPointer()), mResourceGroup.mShaders["deferredPs"]->GetBufferSize() };


    DXGI_FORMAT gBufferFormat = gBuffer->GetFormat();
    psoDesc.NumRenderTargets = 6;
    psoDesc.RTVFormats[0] = gBufferFormat;
    psoDesc.RTVFormats[1] = gBufferFormat;
    psoDesc.RTVFormats[2] = gBufferFormat;
    psoDesc.RTVFormats[3] = gBufferFormat;
    psoDesc.RTVFormats[4] = gBufferFormat;
    psoDesc.RTVFormats[5] = gBufferFormat;

    mResourceGroup.mPSOs["deferredPSO"] = Dx12Device::CreatePSO(&psoDesc);



}

void D3D12App::Update()
{
    UINT bufferNum = Dx12Device::FrameNumToBufferNum(mFrameCount);

    mObjectRotation += 0.001;

    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0, 0, 10);
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationX(0.3 * DirectX::XM_PI);
    DirectX::XMMATRIX rotation2 = DirectX::XMMatrixRotationY(mObjectRotation * DirectX::XM_PI);

    DirectX::XMMATRIX rot = DirectX::XMMatrixMultiply(rotation, rotation2);

    DirectX::XMMATRIX transform = DirectX::XMMatrixMultiply(rot, translation);
    DirectX::XMFLOAT4X4 translationFloat;
    DirectX::XMStoreFloat4x4(&translationFloat, XMMatrixTranspose(transform));

    mResourceGroup.mObjects["box"]->mConstants.worldTransform = translationFloat;

    DirectX::XMMATRIX translation2 = DirectX::XMMatrixTranslation(-5, 0, 20);
    DirectX::XMMATRIX transform2 = DirectX::XMMatrixMultiply(rot, translation2);
    DirectX::XMFLOAT4X4 translationFloat2;
    DirectX::XMStoreFloat4x4(&translationFloat2, XMMatrixTranspose(transform2));
    mResourceGroup.mObjects["box2"]->mConstants.worldTransform = translationFloat2;

    mResourceGroup.mObjects["box"]->UpdateBuffer(bufferNum);
    mResourceGroup.mObjects["box2"]->UpdateBuffer(bufferNum);
    mConstants.UpdateBuffer(bufferNum);
}

void D3D12App::Render()
{
    // This waits for the current frame allocator to finish executing GPU side if it
    // hasn't already. For efficiency it only creates the wait event if the current
    // fence value is below the current frame fence value
    Dx12Device::FlushQueue();

    auto cmdListAlloc = Dx12Device::GetCurrentFrameAllocator();
    auto cmdList = Dx12Device::GetCommandList();
    auto cmdQueue = Dx12Device::GetCommandQueue();

    cmdListAlloc = Dx12Device::GetCurrentFrameAllocator();
    cmdListAlloc->Reset();

    cmdList->Reset(cmdListAlloc.Get(), nullptr);

    mPassGraph.Walk(mFrameCount);

    cmdList->Close();

    ID3D12CommandList* lists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(lists), lists);

    Dx12Device::Present();
    ++mFrameCount;
}

void D3D12App::Run()
{
    // TODO: Put the render loop here so we can run everything from
    // a separate thread
}