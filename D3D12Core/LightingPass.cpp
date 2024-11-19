#include "LightingPass.h"

void LightingPass::PreRender(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();

    PIXBeginEvent(cmdList.Get(), 0, "Lighting");
    PIXBeginEvent(cmdList.Get(), 0, "PreRender");

    UINT bufferNumber = Dx12Device::FrameNumToBufferNum(frameNumber);

    GBuffer* gBuffer = nullptr;
    gBuffer = Dx12Device::GetGBuffer();

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


    // Drawing straight to the back buffer
    D3D12_CPU_DESCRIPTOR_HANDLE views[] = { Dx12Device::GetCurrentBackBufferView() };
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = Dx12Device::GetDepthStencilView();
    cmdList->OMSetRenderTargets(1, views, true, &dsv);

    // Transition to render target state
    CD3DX12_RESOURCE_BARRIER presentToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(Dx12Device::GetCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &presentToRenderTarget);



    float color[] = { 0.2, 0.2, 0.5, 1 };
    cmdList->ClearRenderTargetView(Dx12Device::GetCurrentBackBufferView(), color, 1, &rect);
    //cmdList->ClearDepthStencilView(Dx12Device::GetDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);



    // TODO need to get the heap form the gbufer
    // The last heap is the CbvSrvUav heap, only bind it for now since the other
    // heaps dont have shader visible flags
    //auto heaps = Dx12Device::GetDescriptorHeaps();
    ID3D12DescriptorHeap* heaps[] =
    {
        gBuffer->GetSrvHeap()
    };
    cmdList->SetDescriptorHeaps(1, heaps);

    cmdList->SetGraphicsRootSignature(mResourceGroup->mRootSignatures["lightingRootSig"].Get());

    cmdList->SetGraphicsRootConstantBufferView(0, mConstants->GetResourceForFrame(bufferNumber)->GetGPUVirtualAddress());

    cmdList->SetGraphicsRootDescriptorTable(1, gBuffer->GetGpuSrvHandle());

    cmdList->SetPipelineState(mResourceGroup->mPSOs["deferredLightingPSO"].Get());

    PIXEndEvent(cmdList.Get());
}

void LightingPass::Render(UINT frameNumber)
{
    auto cmdList = Dx12Device::GetCommandList();
    UINT bufferNum = Dx12Device::FrameNumToBufferNum(frameNumber);

    PIXBeginEvent(cmdList.Get(), 0, "Render");

    //for (RenderObject* ref : mRenderObjectRefs)
    //{
        //cmdList->SetGraphicsRootConstantBufferView(1, ref->GetBuffer(bufferNum)->mUploadBuffer->GetGPUVirtualAddress());

        //auto SrvGpuHandle = mResourceGroup->mTextures[ref->mTextureRef].mSrvHandle.mGpuHandle;

        //cmdList->SetGraphicsRootDescriptorTable(2, SrvGpuHandle);

        Mesh* mesh = mResourceGroup->mGeometry["triangle"].get();

        cmdList->IASetVertexBuffers(0, 1, &mesh->mVertexBufferView);
        cmdList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT count = mesh->mVertexCount;
        cmdList->DrawInstanced(count, 1, 0, 0);
    //}

        PIXEndEvent(cmdList.Get());
        PIXEndEvent(cmdList.Get());
}

void LightingPass::PostRender(UINT frameNumber)
{
    // If we end up putting something here and want to add a PIX marker, we need to remove one of the PIXEndEvent
    // calls from the Render function and place it at the end of this function. Otherwise they wont be in the
    // correct order and the pix runtime will freak out

    // We're tacking on a GUI pass after this so we'll wait to transition until the post render of the GUI pass
    /*auto cmdList = Dx12Device::GetCommandList();

    CD3DX12_RESOURCE_BARRIER renderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(Dx12Device::GetCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &renderTargetToPresent);*/
}