#include "ForwardPass.h"

void ForwardPass::PreRender(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();

    UINT bufferNumber = Dx12Device::FrameNumToBufferNum(frameNumber);

    D3D12_VIEWPORT viewPort = Dx12Device::GetViewport();
    D3D12_RECT rect = { 0, 0, static_cast<LONG>(viewPort.Width), static_cast<LONG>(viewPort.Height) };

    cmdList->RSSetViewports(1, &viewPort);
    cmdList->RSSetScissorRects(1, &rect);


    // Drawing straight to the back buffer
    D3D12_CPU_DESCRIPTOR_HANDLE views[] = { Dx12Device::GetCurrentBackBufferView() };
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = Dx12Device::GetDepthStencilView();
    cmdList->OMSetRenderTargets(1, views, true, &dsv);

    // Transition to render target state
    CD3DX12_RESOURCE_BARRIER presentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(Dx12Device::GetCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &presentToRenderTarget);

    

    float color[] = { 0.2f, 0.2f, 0.5f, 1.f };
    cmdList->ClearRenderTargetView(Dx12Device::GetCurrentBackBufferView(), color, 1, &rect);
    cmdList->ClearDepthStencilView(Dx12Device::GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);


    // The last heap is the CbvSrvUav heap, only bind it for now since the other
    // heaps dont have shader visible flags
    auto heaps = Dx12Device::GetDescriptorHeaps();
    cmdList->SetDescriptorHeaps(1, &heaps[2]);

    cmdList->SetGraphicsRootSignature(mResourceGroup->mRootSignatures["mainRootSig"_h].Get());

    cmdList->SetGraphicsRootConstantBufferView(0, mConstants->GetResourceForFrame(bufferNumber)->GetGPUVirtualAddress());

    cmdList->SetPipelineState(mResourceGroup->mPSOs["simplePSO"_h].Get());
}

void ForwardPass::Render(UINT frameNumber)
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

void ForwardPass::PostRender(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();

    CD3DX12_RESOURCE_BARRIER renderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(Dx12Device::GetCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &renderTargetToPresent);
}