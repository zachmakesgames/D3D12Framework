#include "UnlitPass.h"

void UnlitPass::PreRender(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();

    PIXBeginEvent(cmdList.Get(), 0, "Unlit");
    PIXBeginEvent(cmdList.Get(), 0, "PreRender");

    UINT bufferNumber = Dx12Device::FrameNumToBufferNum(frameNumber);

    GBuffer* gBuffer = nullptr;
    gBuffer = Dx12Device::GetGBuffer();

    D3D12_VIEWPORT viewPort = Dx12Device::GetViewport();
    D3D12_RECT rect = { 0, 0, static_cast<LONG>(viewPort.Width), static_cast<LONG>(viewPort.Height) };

    cmdList->RSSetViewports(1, &viewPort);
    cmdList->RSSetScissorRects(1, &rect);


    // Drawing straight to the back buffer
    D3D12_CPU_DESCRIPTOR_HANDLE views[] = { Dx12Device::GetCurrentBackBufferView() };
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = Dx12Device::GetDepthStencilView();
    cmdList->OMSetRenderTargets(1, views, true, &dsv);


    auto heaps = Dx12Device::GetDescriptorHeaps();
    cmdList->SetDescriptorHeaps(1, &heaps[2]);

    cmdList->SetGraphicsRootSignature(mResourceGroup->mRootSignatures["mainRootSig"_h].Get());

    cmdList->SetGraphicsRootConstantBufferView(0, mConstants->GetResourceForFrame(bufferNumber)->GetGPUVirtualAddress());
    cmdList->SetPipelineState(mResourceGroup->mPSOs["simplePSO"_h].Get());

    PIXEndEvent(cmdList.Get());
}

void UnlitPass::Render(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();
    PIXBeginEvent(cmdList.Get(), 0, "Render");

    UINT bufferNum = Dx12Device::FrameNumToBufferNum(frameNumber);

    for (RenderObject* ref : mRenderObjectRefs)
    {

        cmdList->SetGraphicsRootConstantBufferView(1, ref->GetBuffer(bufferNum)->mUploadBuffer->GetGPUVirtualAddress());

        auto SrvGpuHandle = mResourceGroup->mTextures[ref->mTextureRef].mSrvHandle.mGpuHandle;

        cmdList->SetGraphicsRootDescriptorTable(2, SrvGpuHandle);

        Mesh* mesh = mResourceGroup->mGeometry[ref->mMeshRef].get();

        cmdList->IASetVertexBuffers(0, 1, &mesh->mVertexBufferView);

        if (ref->IsInstanced())
        {
            D3D12_VERTEX_BUFFER_VIEW instBufferView = {};
            Buffer* instBuffer = ref->GetInstanceBuffer(bufferNum);
            instBufferView.BufferLocation = instBuffer->mUploadBuffer->GetGPUVirtualAddress();
            instBufferView.SizeInBytes = ref->GetInstBufferSize();
            instBufferView.StrideInBytes = sizeof(RenderItemInstanceValues);

            //returnMesh->mVertexBufferView.BufferLocation = returnMesh->mVertexBuffer->mResource->GetGPUVirtualAddress();
            //returnMesh->mVertexBufferView.SizeInBytes = returnMesh->mVertexCount * sizeof(Vertex);
            //returnMesh->mVertexBufferView.StrideInBytes = sizeof(Vertex);
            // Bind the instance buffer
            cmdList->IASetVertexBuffers(1, 1, &instBufferView);
        }

        cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT count = mesh->mVertexCount;
        cmdList->DrawInstanced(count, ref->mInstanceCount, 0, 0);
    }

    PIXEndEvent(cmdList.Get());
}

void UnlitPass::PostRender(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();
    PIXBeginEvent(cmdList.Get(), 0, "PostRender");



    PIXEndEvent(cmdList.Get());
    PIXEndEvent(cmdList.Get());
}