#include "GuiPass.h"


void GuiPass::PreRender(UINT frameNumber)
{
	auto cmdList = Dx12Device::GetCommandList();
	UINT bufferNumber = Dx12Device::FrameNumToBufferNum(frameNumber);


	auto heaps = Dx12Device::GetDescriptorHeaps();
	cmdList->SetDescriptorHeaps(1, &heaps[3]);

	// TODO: Clear the depth so the GUI is always in front?
}

void GuiPass::Render(UINT frameNumber)
{
	auto cmdList = Dx12Device::GetCommandList();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList.Get());
}

void GuiPass::PostRender(UINT frameNumber)
{
	auto cmdList = Dx12Device::GetCommandList();

	CD3DX12_RESOURCE_BARRIER renderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(Dx12Device::GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	cmdList->ResourceBarrier(1, &renderTargetToPresent);
}