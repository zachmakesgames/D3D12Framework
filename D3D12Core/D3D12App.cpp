#include "D3D12App.h"
#include <random>

D3D12App::D3D12App(HWND window, int swapchainWidth, int swapchainHeight, bool windowed):
	mWindow(window), mWidth(swapchainWidth), mHeight(swapchainHeight), mWindowed(windowed)
{
}

void D3D12App::Init()
{
	Dx12Device::Create();
	Dx12Device::InitSwapchain(mWindow, mWidth, mHeight, mWindowed);

    Dx12Device::InitImGui();

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

    RenderObjectInit boxInit = { "box", "TestPattern", false, 1 };
    RenderObjectInit d20Init = { "d20", "TestPattern", false, 1 };
    
    
    mResourceGroup.mObjects["box"] = std::make_unique<RenderObject>(boxInit);
    mResourceGroup.mObjects["box2"] = std::make_unique<RenderObject>(d20Init);

    // Instanced rendering example with new support for instances built into 
    // RenderObject
    RenderObjectInit d20InstInit = { "d20", "TestPattern", true, 2000 };
    mResourceGroup.mObjects["d20Inst"] = std::make_unique<RenderObject>(d20InstInit);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distro(-200, 200);

    for (int i = 0; i < d20InstInit.instanceCount; ++i)
    {

        int x = distro(gen);
        int y = distro(gen);
        int z = distro(gen);
        //DirectX::XMFLOAT3 offset = instTransforms[i % 12];
        //DirectX::XMMATRIX offsetMat = DirectX::XMMatrixTranslation(offset.x, offset.y, offset.z);
        DirectX::XMMATRIX offsetMat = DirectX::XMMatrixTranslation(x, y, z);

        DirectX::XMFLOAT4X4 offsetMat4;
        //DirectX::XMStoreFloat4x4(&offsetMat4, DirectX::XMMatrixTranspose(offsetMat));
        DirectX::XMStoreFloat4x4(&offsetMat4, offsetMat);


        mResourceGroup.mObjects["d20Inst"]->mInstanceValues[i].instanceTransform = offsetMat4;
    }

    for (int i = 0; i < Dx12Device::GetSwapchainBufferCount(); ++i)
    {
        mResourceGroup.mObjects["d20Inst"]->UpdateInstanceBuffer(i);
    }
    

    // A basic forward rendering pass
    mPasses["mainPass"] = new ForwardPass(&mResourceGroup, &mConstants);
    mPasses["mainPass"]->mPassName = "Forward pass";

    mPasses["mainPass"]->RegisterRenderObject(mResourceGroup.mObjects["box"].get());
    mPasses["mainPass"]->RegisterRenderObject(mResourceGroup.mObjects["box2"].get());


    // The GBuffer pass for deferred lighting
    mPasses["deferredPass"] = new GBufferPass(&mResourceGroup, &mConstants);
    mPasses["deferredPass"]->mPassName = "Deferred pass";

    mPasses["deferredPass"]->RegisterRenderObject(mResourceGroup.mObjects["box"].get());
    mPasses["deferredPass"]->RegisterRenderObject(mResourceGroup.mObjects["box2"].get());
    mPasses["deferredPass"]->RegisterRenderObject(mResourceGroup.mObjects["d20Inst"].get());

    // The lighting pass for deferred lighting
    mPasses["deferredLightingPass"] = new LightingPass(&mResourceGroup, &mConstants);
    mPasses["deferredLightingPass"]->mPassName = "Deferred lighting pass";

    mPasses["guiPass"] = new GuiPass(&mResourceGroup, &mConstants);
    mPasses["guiPass"]->mPassName = "GUI Pass";

    //mPassGraph.AddPass(mPasses["mainPass"]);          // This would enable forward rendering with no lighting
    mPassGraph.AddPass(mPasses["deferredPass"]);        // These enable deferred rendering with lighting
    mPassGraph.AddPass(mPasses["deferredLightingPass"]);
    mPassGraph.AddPass(mPasses["guiPass"]);

    DirectX::XMMATRIX ident = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT4X4 identF;

    DirectX::XMStoreFloat4x4(&identF, ident);
    
    DirectX::XMFLOAT4X4 projection = identF;
    DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, 300.f / 300.f, 1.0f, 1000.0f);

    //DirectX::XMStoreFloat4x4(&projection, XMMatrixTranspose(projMat));
    DirectX::XMStoreFloat4x4(&projection, projMat);

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
    //DirectX::XMStoreFloat4x4(&translationFloat, XMMatrixTranspose(transform));
    DirectX::XMStoreFloat4x4(&translationFloat, transform);


    mResourceGroup.mObjects["box"]->mConstants.worldTransform = translationFloat;

    DirectX::XMMATRIX translation2 = DirectX::XMMatrixTranslation(-5, 0, 15);
    DirectX::XMMATRIX transform2 = DirectX::XMMatrixMultiply(rot, translation2);
    DirectX::XMFLOAT4X4 translationFloat2;
    //DirectX::XMStoreFloat4x4(&translationFloat2, XMMatrixTranspose(transform2));
    DirectX::XMStoreFloat4x4(&translationFloat2, transform2);

    mResourceGroup.mObjects["box2"]->mConstants.worldTransform = translationFloat2;



    cmdList->Close();

    ID3D12CommandList* lists[] = { cmdList.Get() };
    cmdQueue->ExecuteCommandLists(_countof(lists), lists);
    Dx12Device::FlushQueue(true);

    mInited = true;
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
    mResourceGroup.mGeometry["d20"] = Mesh::LoadMeshFromObj("../../../Resources/Models/D20.obj");

    mResourceGroup.mGeometry["triangle"] = Mesh::CreateMesh(sFullScreenTriangle, 3);



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
    GBuffer* gBuffer = nullptr;
    gBuffer = Dx12Device::GetGBuffer();

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        // Instance transform for per instance draws. To pass a 4x4 matrix, we need to specify it as 4 separate float4s
        // But we can access them as a single float4x4 in HLSL with just INSTTRANSFORM
        { "INSTTRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        { "INSTTRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        { "INSTTRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
        { "INSTTRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
    psoDesc.pRootSignature = mResourceGroup.mRootSignatures["mainRootSig"].Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["simpleVs"]->GetBufferPointer()), mResourceGroup.mShaders["simpleVs"]->GetBufferSize()};
    psoDesc.PS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["simplePs"]->GetBufferPointer()), mResourceGroup.mShaders["simplePs"]->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
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

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();


    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Hello, world!");
    ImGui::SetWindowFontScale(2.f);
    ImGui::Text("Testing 1 2 3");

    ImGui::End();

    mInputState.PollKeyboard();
    mInputState.PollMouse(mWindow);

    auto lastTime = mFrameTimer;
    mFrameTimer = std::chrono::high_resolution_clock::now();
    mFrameDuration = mFrameTimer - lastTime;

    float dt = mFrameDuration.count();
    float cameraSpeed = 0.1;


    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        DirectX::XMFLOAT3 cameraForward = mCamera.GetForwardVector();
        DirectX::XMFLOAT3 cameraRight = mCamera.GetRightVector();
        DirectX::XMFLOAT3 cameraUp = mCamera.GetUpVector();

        DirectX::XMVECTOR cameraForwardV = DirectX::XMLoadFloat3(&cameraForward);
        DirectX::XMVECTOR cameraRightV = DirectX::XMLoadFloat3(&cameraRight);
        DirectX::XMVECTOR cameraUpV = DirectX::XMLoadFloat3(&cameraUp);

        if (mInputState.IsKeyDown("W"))
        {
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraForwardV, -1 * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);

        }
        if (mInputState.IsKeyDown("S"))
        {
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraForwardV, 1 * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);
        }
        if (mInputState.IsKeyDown("D"))
        {

            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraRightV, -1 * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);
        }
        if (mInputState.IsKeyDown("A"))
        {
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraRightV, 1 * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);
        }

        if (mInputState.IsKeyDown("R"))
        {
            mCamera.SetPosition(DirectX::XMFLOAT3(0, 0, 0));
            mCamera.SetYaw(0);
            mCamera.SetPitch(0);
        }
    }

    if (!ImGui::GetIO().WantCaptureMouse)
    {
        if (mInputState.IsKeyDown("LEFTMOUSE"))
        {
            POINT mousePos = mInputState.GetMouseChange();
            if (mousePos.x != 0 || mousePos.y != 0)
            {
                float x = (float)mousePos.x / (float)mWidth;
                float y = (float)mousePos.y / (float)mHeight;

                // The camera movement is a bit weird, if we multiply it
                // by the frame time then it jumps wildly and randomly with
                // only minor differences in mouse position. The problem seems
                // to be limited by not multiplying by dt, but it can still be
                // noticed subtly

                x = x * -1.f * 40.f;
                y = y * -1.f * 40.f;
                mCamera.AddPitch(DirectX::XMConvertToRadians(y));
                mCamera.AddYaw(DirectX::XMConvertToRadians(x));

                //x = x * DirectX::XM_PI * dt * -1;
                //y = y * DirectX::XM_PI * dt * -1;
                //mCamera.AddPitch(y);
                //mCamera.AddYaw(x);

            }
        }
    }

    UINT bufferNum = Dx12Device::FrameNumToBufferNum(mFrameCount);

    mObjectRotation += 0.001 * dt;

    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0, 0, 10);
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationX(0.3 * DirectX::XM_PI);
    DirectX::XMMATRIX rotation2 = DirectX::XMMatrixRotationY(mObjectRotation * DirectX::XM_PI);

    DirectX::XMMATRIX rot = DirectX::XMMatrixMultiply(rotation, rotation2);

    DirectX::XMMATRIX transform = DirectX::XMMatrixMultiply(rot, translation);
    DirectX::XMFLOAT4X4 translationFloat;
    //DirectX::XMStoreFloat4x4(&translationFloat, XMMatrixTranspose(transform));
    DirectX::XMStoreFloat4x4(&translationFloat, transform);

    mResourceGroup.mObjects["box"]->mConstants.worldTransform = translationFloat;

    DirectX::XMMATRIX translation2 = DirectX::XMMatrixTranslation(-5, 0, 20);
    DirectX::XMMATRIX transform2 = DirectX::XMMatrixMultiply(rot, translation2);
    DirectX::XMFLOAT4X4 translationFloat2;
    //DirectX::XMStoreFloat4x4(&translationFloat2, XMMatrixTranspose(transform2));
    DirectX::XMStoreFloat4x4(&translationFloat2, transform2);

    mResourceGroup.mObjects["box2"]->mConstants.worldTransform = translationFloat2;

    mResourceGroup.mObjects["box"]->UpdateBuffer(bufferNum);
    mResourceGroup.mObjects["box2"]->UpdateBuffer(bufferNum);
    mConstants.UpdateBuffer(bufferNum);

    D3D12_RECT viewport = Dx12Device::GetViewportSize();

    float aspect = (float)viewport.right / (float)viewport.bottom;

    DirectX::XMFLOAT4X4 projection;
    DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspect, 1.0f, 1000.0f);

    //DirectX::XMStoreFloat4x4(&projection, XMMatrixTranspose(projMat));
    DirectX::XMStoreFloat4x4(&projection, projMat);

    mConstants.mWorldConstants.mViewMat = mCamera.GetViewMatrix();


    mConstants.mWorldConstants.mProjMat = projection;
    mConstants.UpdateBuffer(bufferNum);
}

void D3D12App::Render()
{
    ImGui::Render();

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
    mRunning = true;

    while (mRunning)
    {
        if (mResizeQueue.size() > 0)
        {
            int size = mResizeQueue.size();
            DirectX::XMINT2 newSize;
            for (int i = 0; i < size; ++i)
            {
                newSize = mResizeQueue.front();
                mResizeQueue.pop();
            }

            Dx12Device::Resize(newSize.x, newSize.y);
        }
        Update();
        Render();
    }
}

void D3D12App::Stop()
{
    mRunning = false;
}

void D3D12App::Resize(UINT newWidth, UINT newHeight)
{
    DirectX::XMINT2 newSize(newWidth, newHeight);
    mResizeQueue.push(newSize);
}

void D3D12App::PollInputs()
{

}

void D3D12App::LoadScene(Scene* newScene)
{

}