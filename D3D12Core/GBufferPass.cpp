#include "GBufferPass.h"


void GBufferPass::PreRender(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();
    GBuffer* gBuffer = Dx12Device::GetGBuffer();

    UINT bufferNumber = Dx12Device::FrameNumToBufferNum(frameNumber);

    D3D12_RECT rect = Dx12Device::GetViewportSize();

    D3D12_VIEWPORT viewPort = {};
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = rect.right;
    viewPort.Height = rect.bottom;
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;

    cmdList->RSSetViewports(1, &viewPort);
    cmdList->RSSetScissorRects(1, &rect);

    // Transition the GBuffer resources
    CD3DX12_RESOURCE_BARRIER rtvTransitions[6] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(0), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(1), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(2), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(3), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(4), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(5), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET),
    };
    cmdList->ResourceBarrier(6, rtvTransitions);


    // Draw to the GBuffer RTs instead of straight to the back buffer
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandles[6] =
    {
        gBuffer->GetCpuRtvHandle(0),    // Might need to start these offsets at 1 instead of 0
        gBuffer->GetCpuRtvHandle(1),
        gBuffer->GetCpuRtvHandle(2),
        gBuffer->GetCpuRtvHandle(3),
        gBuffer->GetCpuRtvHandle(4),
        gBuffer->GetCpuRtvHandle(5),
    };

    D3D12_CPU_DESCRIPTOR_HANDLE dsv = Dx12Device::GetDepthStencilView();
    cmdList->OMSetRenderTargets(6, rtvHandles, true, &dsv);


    float color[] = { 0, 0, 0, 0 };
    cmdList->ClearRenderTargetView(rtvHandles[0], color, 1, &rect);
    cmdList->ClearRenderTargetView(rtvHandles[1], color, 1, &rect);
    cmdList->ClearRenderTargetView(rtvHandles[2], color, 1, &rect);
    cmdList->ClearRenderTargetView(rtvHandles[3], color, 1, &rect);
    cmdList->ClearRenderTargetView(rtvHandles[4], color, 1, &rect);
    cmdList->ClearRenderTargetView(rtvHandles[5], color, 1, &rect);
    cmdList->ClearDepthStencilView(Dx12Device::GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);


    // The last heap is the CbvSrvUav heap, only bind it for now since the other
    // heaps dont have shader visible flags
    auto heaps = Dx12Device::GetDescriptorHeaps();
    cmdList->SetDescriptorHeaps(1, &heaps[2]);

    cmdList->SetGraphicsRootSignature(mResourceGroup->mRootSignatures["mainRootSig"].Get());

    cmdList->SetGraphicsRootConstantBufferView(0, mConstants->GetResourceForFrame(bufferNumber)->GetGPUVirtualAddress());

    cmdList->SetPipelineState(mResourceGroup->mPSOs["deferredPSO"].Get());
}

void GBufferPass::Render(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();
    UINT bufferNum = Dx12Device::FrameNumToBufferNum(frameNumber);

    for (RenderObject* ref : mRenderObjectRefs)
    {
        cmdList->SetGraphicsRootConstantBufferView(1, ref->GetBuffer(bufferNum)->mUploadBuffer->GetGPUVirtualAddress());

        auto SrvGpuHandle = mResourceGroup->mTextures[ref->mTextureRef].mSrvHandle.mGpuHandle;

        cmdList->SetGraphicsRootDescriptorTable(2, SrvGpuHandle);

        Mesh* mesh = mResourceGroup->mGeometry[ref->mMeshRef].get();

        cmdList->IASetVertexBuffers(0, 1, &mesh->mVertexBufferView);
        cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT count = mesh->mVertexCount;
        cmdList->DrawInstanced(count, 1, 0, 0);
    }
}

void GBufferPass::PostRender(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();
    GBuffer* gBuffer = Dx12Device::GetGBuffer();

    // Transition the GBuffer resources
    CD3DX12_RESOURCE_BARRIER rtvTransitions[6] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(0), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(1), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(2), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(3), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(4), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
        CD3DX12_RESOURCE_BARRIER::Transition(gBuffer->GetResource(5), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
    };
    cmdList->ResourceBarrier(6, rtvTransitions);
}