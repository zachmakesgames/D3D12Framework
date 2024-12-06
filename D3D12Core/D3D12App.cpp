#include "D3D12App.h"
#include <random>
#include "Renderer/Utils.h"

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

    RenderObjectInit boxInit = { "box"_h, "TestPattern"_h, false, 1 };
    RenderObjectInit d20Init = { "d20"_h, "TestPattern"_h, false, 1 };

    RenderObjectInit gizmoInit{ "gizmoArrow"_h, "TestPattern"_h, true, 3 };
    RenderObjectInit debugLineInit{ "debugLine"_h, "TestPattern"_h, true, 1 };
    RenderObjectInit debugCubeInit{ "debugCube"_h, "TestPattern"_h, true, 2 };
    
    mResourceGroup.mObjects["box"_h] = std::make_unique<RenderObject>(boxInit);
    mResourceGroup.mObjects["box2"_h] = std::make_unique<RenderObject>(d20Init);

    mResourceGroup.mObjects["gizmo"_h] = std::make_unique<RenderObject>(gizmoInit);
    mResourceGroup.mObjects["debugLine"_h] = std::make_unique<RenderObject>(debugLineInit);
    mResourceGroup.mObjects["debugCube"_h] = std::make_unique<RenderObject>(debugCubeInit);

    // Instanced rendering example with new support for instances built into 
    // RenderObject
    RenderObjectInit d20InstInit = { "d20"_h, "TestPattern"_h, true, 100000 };
    mResourceGroup.mObjects["d20Inst"_h] = std::make_unique<RenderObject>(d20InstInit);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distro(-600, 600);

    for (UINT i = 0; i < d20InstInit.instanceCount; ++i)
    {

        float x = (float)distro(gen);
        float y = (float)distro(gen);
        float z = (float)distro(gen);
        DirectX::XMMATRIX offsetMat = DirectX::XMMatrixTranslation(x, y, z);

        DirectX::XMFLOAT4X4 offsetMat4;
        DirectX::XMStoreFloat4x4(&offsetMat4, offsetMat);


        mResourceGroup.mObjects["d20Inst"_h]->mInstanceValues[i].instanceTransform = offsetMat4;
    }


    // All this just to get a couple of arrows pointed in the right direction
    // DirectX Math, what is wrong with you?
    DirectX::XMFLOAT4 xAxis = DirectX::XMFLOAT4(1, 0, 0, 1);
    DirectX::XMFLOAT4 zAxis = DirectX::XMFLOAT4(0, 0, 1, 1);

    DirectX::XMMATRIX gizmoOffset = DirectX::XMMatrixTranslation(0, 0, 10);

    DirectX::XMVECTOR xAxisVector = DirectX::XMLoadFloat4(&xAxis);
    DirectX::XMMATRIX gizmoXRotation = DirectX::XMMatrixRotationAxis(xAxisVector, DirectX::XMConvertToRadians(90));

    DirectX::XMFLOAT4X4 xAxisRotation;
    DirectX::XMStoreFloat4x4(&xAxisRotation, gizmoXRotation);

    DirectX::XMVECTOR zAxisVector = DirectX::XMLoadFloat4(&zAxis);
    DirectX::XMMATRIX gizmoZRotation = DirectX::XMMatrixRotationAxis(zAxisVector, DirectX::XMConvertToRadians(-90));

    DirectX::XMFLOAT4X4 zAxisRotation;
    DirectX::XMStoreFloat4x4(&zAxisRotation, gizmoZRotation);

    DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT4X4 identity4x4;
    DirectX::XMStoreFloat4x4(&identity4x4, identity);

    DirectX::XMFLOAT4X4 gizmoOffset4x4;
    DirectX::XMStoreFloat4x4(&gizmoOffset4x4, gizmoOffset);

    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.5, 0.5, 0.5);
    DirectX::XMFLOAT4X4 scale4x4;
    DirectX::XMStoreFloat4x4(&scale4x4, scale);

    DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(5, 0, 0);
    DirectX::XMFLOAT4X4 translateX4x4;
    DirectX::XMStoreFloat4x4(&translateX4x4, translateX);

    DirectX::XMMATRIX translateY = DirectX::XMMatrixTranslation(0, 5, 0);
    DirectX::XMFLOAT4X4 translateY4x4;
    DirectX::XMStoreFloat4x4(&translateY4x4, translateY);

    DirectX::XMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0, 5);
    DirectX::XMFLOAT4X4 translateZ4x4;
    DirectX::XMStoreFloat4x4(&translateZ4x4, translateZ);
    
    
    mResourceGroup.mObjects["gizmo"_h]->mInstanceValues[0].instanceTransform = zAxisRotation;
    mResourceGroup.mObjects["gizmo"_h]->mInstanceValues[0].instanceColor = DirectX::XMFLOAT4(1, 0, 0, 1);
    mResourceGroup.mObjects["gizmo"_h]->mInstanceValues[1].instanceTransform = identity4x4;
    mResourceGroup.mObjects["gizmo"_h]->mInstanceValues[1].instanceColor = DirectX::XMFLOAT4(0, 1, 0, 1);
    mResourceGroup.mObjects["gizmo"_h]->mInstanceValues[2].instanceTransform = xAxisRotation;
    mResourceGroup.mObjects["gizmo"_h]->mInstanceValues[2].instanceColor = DirectX::XMFLOAT4(0, 0, 1, 1);

    mResourceGroup.mObjects["debugLine"_h]->mInstanceValues[0].instanceTransform = identity4x4;
    mResourceGroup.mObjects["debugLine"_h]->mInstanceValues[0].instanceColor = DirectX::XMFLOAT4(1, 1, 1, 1);

    mResourceGroup.mObjects["debugCube"_h]->mInstanceValues[0].instanceTransform = scale4x4;
    mResourceGroup.mObjects["debugCube"_h]->mInstanceValues[0].instanceColor = DirectX::XMFLOAT4(1, 148.f/255.f, 27.f/255.f, 1);
    mResourceGroup.mObjects["debugCube"_h]->mInstanceValues[1].instanceTransform = scale4x4;
    mResourceGroup.mObjects["debugCube"_h]->mInstanceValues[1].instanceColor = DirectX::XMFLOAT4(0, 1.f, 0.f, 1);



    for (int i = 0; i < Dx12Device::GetSwapchainBufferCount(); ++i)
    {
        mResourceGroup.mObjects["d20Inst"_h]->UpdateInstanceBuffer(i);
        mResourceGroup.mObjects["gizmo"_h]->UpdateInstanceBuffer(i);
        mResourceGroup.mObjects["debugLine"_h]->UpdateInstanceBuffer(i);
        mResourceGroup.mObjects["debugCube"_h]->UpdateInstanceBuffer(i);
    }

    
    

    // A basic forward rendering pass
    mPasses["mainPass"] = new ForwardPass(&mResourceGroup, &mConstants);
    mPasses["mainPass"]->mPassName = "Forward pass";

    mPasses["mainPass"]->RegisterRenderObject(mResourceGroup.mObjects["box"_h].get());
    mPasses["mainPass"]->RegisterRenderObject(mResourceGroup.mObjects["box2"_h].get());


    // The GBuffer pass for deferred lighting
    mPasses["deferredPass"] = new GBufferPass(&mResourceGroup, &mConstants);
    mPasses["deferredPass"]->mPassName = "Deferred pass";

    mPasses["deferredPass"]->RegisterRenderObject(mResourceGroup.mObjects["box"_h].get());
    mPasses["deferredPass"]->RegisterRenderObject(mResourceGroup.mObjects["box2"_h].get());
    mPasses["deferredPass"]->RegisterRenderObject(mResourceGroup.mObjects["d20Inst"_h].get());

    // The lighting pass for deferred lighting
    mPasses["deferredLightingPass"] = new LightingPass(&mResourceGroup, &mConstants);
    mPasses["deferredLightingPass"]->mPassName = "Deferred lighting pass";

    mPasses["guiPass"] = new GuiPass(&mResourceGroup, &mConstants);
    mPasses["guiPass"]->mPassName = "GUI Pass";

    mPasses["unlitPass"] = new UnlitPass(&mResourceGroup, &mConstants);
    mPasses["unlitPass"]->mPassName = "Unlit Pass";

    mPasses["debugPass"] = new DebugPass(&mResourceGroup, &mConstants);
    mPasses["debugPass"]->mPassName = "Debug Pass";

    mPasses["unlitPass"]->RegisterRenderObject(mResourceGroup.mObjects["gizmo"_h].get());
    mPasses["debugPass"]->RegisterRenderObject(mResourceGroup.mObjects["debugLine"_h].get());
    mPasses["debugPass"]->RegisterRenderObject(mResourceGroup.mObjects["debugCube"_h].get());

    //mPassGraph.AddPass(mPasses["mainPass"]);          // This would enable forward rendering with no lighting
    mPassGraph.AddPass(mPasses["deferredPass"]);        // These enable deferred rendering with lighting
    mPassGraph.AddPass(mPasses["deferredLightingPass"]);
    mPassGraph.AddPass(mPasses["unlitPass"]);
    mPassGraph.AddPass(mPasses["debugPass"]);
    mPassGraph.AddPass(mPasses["guiPass"]);

    DirectX::XMMATRIX ident = DirectX::XMMatrixIdentity();
    DirectX::XMFLOAT4X4 identF;
    DirectX::XMStoreFloat4x4(&identF, ident);
    
    DirectX::XMFLOAT4X4 projection = identF;
    DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, (float)mWidth / (float)mHeight, 1.0f, 1000.0f);
    DirectX::XMStoreFloat4x4(&projection, projMat);

    mConstants.mWorldConstants.mProjMat = projection;
    mConstants.mWorldConstants.mViewMat =  identF;
    
    mConstants.Init();


    // Push the box out in front of the camera (Z axis) and slightly to the right (X axis)
    // so we can see the projection is working right.
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0, 0, 10);
    
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationX(0.3 * DirectX::XM_PI);
    DirectX::XMMATRIX rotation2 = DirectX::XMMatrixRotationY(0.3 * DirectX::XM_PI);

    DirectX::XMMATRIX rot = DirectX::XMMatrixMultiply(rotation, rotation2);

    DirectX::XMMATRIX transform = DirectX::XMMatrixMultiply(rot, translation);
    DirectX::XMFLOAT4X4 translationFloat;
    DirectX::XMStoreFloat4x4(&translationFloat, transform);


    mResourceGroup.mObjects["box"_h]->mConstants.worldTransform = translationFloat;

    DirectX::XMMATRIX translation2 = DirectX::XMMatrixTranslation(-5, 0, 15);
    DirectX::XMMATRIX transform2 = DirectX::XMMatrixMultiply(rot, translation2);
    DirectX::XMFLOAT4X4 translationFloat2;
    DirectX::XMStoreFloat4x4(&translationFloat2, transform2);

    mResourceGroup.mObjects["box2"_h]->mConstants.worldTransform = translationFloat2;



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

    mResourceGroup.mRootSignatures["mainRootSig"_h] = Dx12Device::CreateRootSignature(&mainRootSig);


    CD3DX12_DESCRIPTOR_RANGE gBufferSrvTable;
    gBufferSrvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 0, 0);

    CD3DX12_ROOT_PARAMETER lightingRootParam[2];
    lightingRootParam[0].InitAsConstantBufferView(0);
    lightingRootParam[1].InitAsDescriptorTable(1, &gBufferSrvTable, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_ROOT_SIGNATURE_DESC lightingRootSig(2, lightingRootParam,
        (UINT)samplers.size(), samplers.data(),
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    mResourceGroup.mRootSignatures["lightingRootSig"_h] = Dx12Device::CreateRootSignature(&lightingRootSig);
}

void D3D12App::CreateGeometry()
{
    mResourceGroup.mGeometry["box"_h] = Mesh::LoadMeshFromObj("../../../Resources/Models/box.obj");
    mResourceGroup.mGeometry["d20"_h] = Mesh::LoadMeshFromObj("../../../Resources/Models/D20.obj");
    mResourceGroup.mCollisionGeometry["d20"_h] = Physics::CollisionMesh::LoadMeshFromObj("../../../Resources/Models/D20.obj");

    mResourceGroup.mGeometry["gizmoArrow"_h] = Mesh::LoadMeshFromObj("../../../Resources/Models/YArrow.obj");

    mResourceGroup.mGeometry["triangle"_h] = Mesh::CreateMesh(sFullScreenTriangle, 3);
    mResourceGroup.mGeometry["debugLine"_h] = Mesh::CreateMesh(sLine, 2);
    mResourceGroup.mGeometry["debugCube"_h] = Mesh::CreateMesh(sCube, sizeof(sCube)/sizeof(Vertex));



    // TODO: Why is this here? Where does this go? How does a computer even work?!
    mResourceGroup.mTextures["TestPattern"_h] = Texture("TestPattern", L"../../../Resources/Textures/TestPattern.dds");
    HRESULT hr = Dx12Device::LoadTextureFromDDSFile(&mResourceGroup.mTextures["TestPattern"_h]);
    if (FAILED(hr))
    {
        OutputDebugString(L"Failed to load DDS texture!\r\n");
    }
}

// TODO: Leave this for only very common shaders, allow GraphicsPasses to create their own resources
void D3D12App::CreateShaders()
{
    mResourceGroup.mShaders["simpleVs"_h] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/test.hlsl", nullptr, L"vsMain", L"vs_6_6");
    mResourceGroup.mShaders["simplePs"_h] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/test.hlsl", nullptr, L"psMain", L"ps_6_6");

    mResourceGroup.mShaders["deferredVs"_h] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/deferred.hlsl", nullptr, L"vsMain", L"vs_6_6");
    mResourceGroup.mShaders["deferredPs"_h] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/deferred.hlsl", nullptr, L"psMain", L"ps_6_6");

    mResourceGroup.mShaders["deferredLightingVs"_h] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/deferredlighting.hlsl", nullptr, L"vsMain", L"vs_6_6");
    mResourceGroup.mShaders["deferredLightingPs"_h] = D3dUtils::Dxc3CompileShader(L"../../../Resources/Shaders/deferredlighting.hlsl", nullptr, L"psMain", L"ps_6_6");
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
        { "INSTCOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

    psoDesc.InputLayout = { inputLayout.data(), (UINT)inputLayout.size() };
    psoDesc.pRootSignature = mResourceGroup.mRootSignatures["mainRootSig"_h].Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["simpleVs"_h]->GetBufferPointer()), mResourceGroup.mShaders["simpleVs"_h]->GetBufferSize()};
    psoDesc.PS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["simplePs"_h]->GetBufferPointer()), mResourceGroup.mShaders["simplePs"_h]->GetBufferSize() };
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

    mResourceGroup.mPSOs["simplePSO"_h] = Dx12Device::CreatePSO(&psoDesc);

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    

    mResourceGroup.mPSOs["debugPSO"_h] = Dx12Device::CreatePSO(&psoDesc);

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.pRootSignature = mResourceGroup.mRootSignatures["lightingRootSig"_h].Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["deferredLightingVs"_h]->GetBufferPointer()), mResourceGroup.mShaders["deferredLightingVs"_h]->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["deferredLightingPs"_h]->GetBufferPointer()), mResourceGroup.mShaders["deferredLightingPs"_h]->GetBufferSize() };

    psoDesc.DepthStencilState.DepthEnable = false;
    mResourceGroup.mPSOs["deferredLightingPSO"_h] = Dx12Device::CreatePSO(&psoDesc);

    psoDesc.DepthStencilState.DepthEnable = true;
    psoDesc.pRootSignature = mResourceGroup.mRootSignatures["mainRootSig"_h].Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["deferredVs"_h]->GetBufferPointer()), mResourceGroup.mShaders["deferredVs"_h]->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<BYTE*>(mResourceGroup.mShaders["deferredPs"_h]->GetBufferPointer()), mResourceGroup.mShaders["deferredPs"_h]->GetBufferSize() };
    

    DXGI_FORMAT gBufferFormat = gBuffer->GetFormat();
    psoDesc.NumRenderTargets = 6;
    psoDesc.RTVFormats[0] = gBufferFormat;
    psoDesc.RTVFormats[1] = gBufferFormat;
    psoDesc.RTVFormats[2] = gBufferFormat;
    psoDesc.RTVFormats[3] = gBufferFormat;
    psoDesc.RTVFormats[4] = gBufferFormat;
    psoDesc.RTVFormats[5] = gBufferFormat;

    mResourceGroup.mPSOs["deferredPSO"_h] = Dx12Device::CreatePSO(&psoDesc);



}

void D3D12App::Update()
{
    UINT bufferNum = Dx12Device::FrameNumToBufferNum(mFrameCount);

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();

    // Need to wrap this in a mutex lock since we're running our render thread
    // on a separate thread from the windows message pump. Things get a little
    // non-newtonian if we don't (and I mean we get random assertions)
    Dx12Device::GetImGuiIoMutex()->lock();
    ImGui::NewFrame();
    Dx12Device::GetImGuiIoMutex()->unlock();

    mFrameTimer.Tick();

    float dt = (float)mFrameTimer.GetFrameTime();
    float cameraSpeed = 0.06f;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);

    std::string windowSize = "";
    windowSize += std::to_string((int)Dx12Device::GetViewport().Width);
    windowSize += "x";
    windowSize += std::to_string((int)Dx12Device::GetViewport().Height);

    std::string frameTimeStr = std::to_string(1000.f/(mFrameTimer.GetAverageFrameTime()));

    std::string cameraPosStr = "";
    cameraPosStr += std::to_string(mCamera.GetCameraPosition().x) + ", ";
    cameraPosStr += std::to_string(mCamera.GetCameraPosition().y) + ", ";
    cameraPosStr += std::to_string(mCamera.GetCameraPosition().z);

    std::string cameraFwdStr = "";
    cameraFwdStr += std::to_string(mCamera.GetForwardVector().x) + ", ";
    cameraFwdStr += std::to_string(mCamera.GetForwardVector().y) + ", ";
    cameraFwdStr += std::to_string(mCamera.GetForwardVector().z);

    ImGui::Begin("Debug Info");
    ImGui::SetWindowFontScale(2.f);
    ImGui::Text("Back buffer dimensions:");
    ImGui::Text(windowSize.c_str());
    ImGui::Text("Frame time:");
    ImGui::Text(frameTimeStr.c_str());
    ImGui::Text("Camera Position");
    ImGui::Text(cameraPosStr.c_str());
    ImGui::Text("Camera Forward:");
    ImGui::Text(cameraFwdStr.c_str());
    ImGui::Text("Ray intersects triangle?");
    ImGui::Text(std::to_string(mRayIntersectsTriangle).c_str());
    if (mRayIntersectsTriangle)
    {
        ImGui::Text("Hit position:");
        ImGui::Text(D3dUtils::VectorToString(mHitPos).c_str());
    }
    ImGui::End();

    mInputState.PollKeyboard();
    mInputState.PollMouse(mWindow);

    D3D12_VIEWPORT viewport = Dx12Device::GetViewport();

    if (!ImGui::GetIO().WantCaptureMouse)
    {
        if (mInputState.IsKeyDown("LEFTMOUSE"))
        {
            POINT mousePos = mInputState.GetMouseChange();
            if (mousePos.x != 0 || mousePos.y != 0)
            {
                float x = (float)mousePos.x / (float)viewport.Width;
                float y = (float)mousePos.y / (float)viewport.Height;

                // The camera movement is a bit weird, if we multiply it
                // by the frame time then it jumps wildly and randomly with
                // only minor differences in mouse position. The problem seems
                // to be limited by not multiplying by dt, but it can still be
                // noticed subtly

                x = x * -1.f * 80.f;
                y = y * -1.f * 80.f;
                mCamera.AddPitch(DirectX::XMConvertToRadians(y));
                mCamera.AddYaw(DirectX::XMConvertToRadians(x));

                //x = x * DirectX::XM_PI * dt * -1;
                //y = y * DirectX::XM_PI * dt * -1;
                //mCamera.AddPitch(y);
                //mCamera.AddYaw(x);

            }
        }

       
    }

    bool doScreenSpaceProjection = false;

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
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraForwardV, -1.f * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);

        }
        if (mInputState.IsKeyDown("S"))
        {
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraForwardV, 1.f * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);
        }
        if (mInputState.IsKeyDown("D"))
        {
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraRightV, -1.f * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);
        }
        if (mInputState.IsKeyDown("A"))
        {
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraRightV, 1.f * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);
        }
        if (mInputState.IsKeyDown("E"))
        {
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraUpV, -1.f * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);
        }
        if (mInputState.IsKeyDown("Q"))
        {
            DirectX::XMVECTOR cameraOffset = DirectX::XMVectorScale(cameraUpV, 1.f * cameraSpeed * dt);
            mCamera.AddPosition(cameraOffset);
        }

        if (mInputState.IsKeyDown("R"))
        {
            mCamera.SetPosition(DirectX::XMFLOAT3(0, 0, 0));
            mCamera.SetYaw(0);
            mCamera.SetPitch(0);
        }

        if (mInputState.IsKeyDown("L"))
        {
            doScreenSpaceProjection = true;
        }
    }

    float aspect = viewport.Width/viewport.Height;

    DirectX::XMFLOAT4X4 projection;
    DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, aspect, .1f, 1000.0f);
    DirectX::XMStoreFloat4x4(&projection, projMat);

    DirectX::XMFLOAT4X4 viewMat4 = mCamera.GetViewMatrix();
    DirectX::XMMATRIX viewMat = DirectX::XMLoadFloat4x4(&viewMat4);

    DirectX::XMFLOAT4 worldSpaceMouse = DirectX::XMFLOAT4(0, 0, 0, 0);

    POINT screenSpaceMouse = mInputState.GetMousePosition();

    DirectX::XMFLOAT4 clipSpaceMouse = DirectX::XMFLOAT4((float)screenSpaceMouse.x, (float)screenSpaceMouse.y, 1000.f, 1.f);
    DirectX::XMVECTOR clipSpaceMouseV = DirectX::XMLoadFloat4(&clipSpaceMouse);


    // Go from clip space to NDC, not useful now but maybe later
    //mouse_x = (2.f * (mouse_x / (float)viewport.right)) - 1.f;
    //mouse_y = (2.f * (mouse_y / (float)viewport.bottom)) - 1.f;

    // Built in unproject requires the mouse position in clip space, not NDC
    DirectX::XMVECTOR unproj = DirectX::XMVector3Unproject(clipSpaceMouseV, 0.f, 0.f, viewport.Width, viewport.Height, 0.1f, 1000.f, projMat, viewMat, DirectX::XMMatrixIdentity());
    DirectX::XMStoreFloat4(&worldSpaceMouse, unproj);



    DirectX::XMFLOAT3 cameraPos = mCamera.GetCameraPosition();
    DirectX::XMFLOAT3 mousePos = DirectX::XMFLOAT3(worldSpaceMouse.x, worldSpaceMouse.y, worldSpaceMouse.z);


    DirectX::XMMATRIX cubeScale = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
    DirectX::XMMATRIX cubeScale2 = DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f);
    DirectX::XMMATRIX cubePos = DirectX::XMMatrixTranslation(cameraPos.x, cameraPos.y, cameraPos.z);
    DirectX::XMMATRIX mouseCubePos = DirectX::XMMatrixTranslation(mousePos.x, mousePos.y, mousePos.z);

    DirectX::XMMATRIX cubeTransform = DirectX::XMMatrixMultiply(cubeScale, cubePos);
    DirectX::XMFLOAT4X4 cubeTransform4x4;

    DirectX::XMMATRIX cubeTransform2 = DirectX::XMMatrixMultiply(cubeScale2, mouseCubePos);
    DirectX::XMFLOAT4X4 cubeTransform2_4x4;

    DirectX::XMStoreFloat4x4(&cubeTransform4x4, cubeTransform);
    DirectX::XMStoreFloat4x4(&cubeTransform2_4x4, cubeTransform2);



    if (doScreenSpaceProjection)
    {
        mResourceGroup.mObjects["debugCube"_h]->mInstanceValues[0].instanceTransform = cubeTransform4x4;
        mResourceGroup.mObjects["debugCube"_h]->mInstanceValues[1].instanceTransform = cubeTransform2_4x4;
        for (int i = 0; i < Dx12Device::GetSwapchainBufferCount(); ++i)
        {
            mResourceGroup.mObjects["debugCube"_h]->UpdateInstanceBuffer(i);
        }


        DirectX::XMFLOAT3 cameraPosx = DirectX::XMFLOAT3(cameraPos.x, cameraPos.y, cameraPos.z);
        DirectX::XMFLOAT3 mousePosx = DirectX::XMFLOAT3(mousePos.x, mousePos.y, mousePos.z);

        mResourceGroup.mGeometry["debugLine"_h]->mVertexData[0].position = cameraPos;
        mResourceGroup.mGeometry["debugLine"_h]->mVertexData[1].position = mousePos;
        mResourceGroup.mGeometry["debugLine"_h]->UpdateBuffer();

        DirectX::XMMATRIX ident = DirectX::XMMatrixIdentity();
        DirectX::XMVECTOR cameraPosV = DirectX::XMLoadFloat3(&cameraPosx);
        DirectX::XMVECTOR mousePosV = DirectX::XMLoadFloat3(&mousePosx);

        DirectX::XMVECTOR rayDir = DirectX::XMVectorSubtract(mousePosV, cameraPosV);


        // Ray intersection test against the spinning d20 object
        DirectX::XMMATRIX d20Transform = DirectX::XMLoadFloat4x4(&mResourceGroup.mObjects["box2"_h]->mConstants.worldTransform);
        mRayIntersectsTriangle = mResourceGroup.mCollisionGeometry["d20"_h]->DoesRayIntersect(cameraPosV, rayDir, d20Transform, &mHitPos);
    }


    mObjectRotation += 0.001f * dt;

    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0, 20, 10);
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationX(0.3 * DirectX::XM_PI);
    DirectX::XMMATRIX rotation2 = DirectX::XMMatrixRotationY(mObjectRotation * DirectX::XM_PI);

    DirectX::XMMATRIX rot = DirectX::XMMatrixMultiply(rotation, rotation2);

    DirectX::XMMATRIX transform = DirectX::XMMatrixMultiply(rot, translation);
    DirectX::XMFLOAT4X4 translationFloat;
    DirectX::XMStoreFloat4x4(&translationFloat, transform);

    mResourceGroup.mObjects["box"_h]->mConstants.worldTransform = translationFloat;

    DirectX::XMMATRIX translation2 = DirectX::XMMatrixTranslation(-5, 0, 20);
    DirectX::XMMATRIX transform2 = DirectX::XMMatrixMultiply(rot, translation2);
    DirectX::XMFLOAT4X4 translationFloat2;
    DirectX::XMStoreFloat4x4(&translationFloat2, transform2);

    mResourceGroup.mObjects["box2"_h]->mConstants.worldTransform = translationFloat2;

    mResourceGroup.mObjects["box"_h]->UpdateBuffer(bufferNum);
    mResourceGroup.mObjects["box2"_h]->UpdateBuffer(bufferNum);
    mConstants.UpdateBuffer(bufferNum);


    mConstants.mWorldConstants.mViewMat = mCamera.GetViewMatrix();
    mConstants.mWorldConstants.mProjMat = projection;
    mConstants.mWorldConstants.mCameraPosition = mCamera.GetCameraPosition();
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
            UINT size = (UINT)mResizeQueue.size();
            DirectX::XMINT2 newSize;
            for (UINT i = 0; i < size; ++i)
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