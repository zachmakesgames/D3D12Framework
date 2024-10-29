#include "GuiPass.h"


void GuiPass::PreRender(UINT frameNumber)
{
	auto cmdList = Dx12Device::GetCommandList();
	UINT bufferNumber = Dx12Device::FrameNumToBufferNum(frameNumber);

	PIXBeginEvent(cmdList.Get(), 0, "UI");
	PIXBeginEvent(cmdList.Get(), 0, "PreRender");

	auto heaps = Dx12Device::GetDescriptorHeaps();
	cmdList->SetDescriptorHeaps(1, &heaps[3]);

	PIXEndEvent(cmdList.Get());
}

void GuiPass::Render(UINT frameNumber)
{
	auto cmdList = Dx12Device::GetCommandList();
	PIXBeginEvent(cmdList.Get(), 0, "Render");

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList.Get());

	PIXEndEvent(cmdList.Get());
}

void GuiPass::PostRender(UINT frameNumber)
{
	auto cmdList = Dx12Device::GetCommandList();

	PIXBeginEvent(cmdList.Get(), 0, "PostRender");

	CD3DX12_RESOURCE_BARRIER renderTargetToPresent = CD3DX12_RESOURCE_BARRIER::Transition(Dx12Device::GetCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	cmdList->ResourceBarrier(1, &renderTargetToPresent);
	
	PIXEndEvent(cmdList.Get());
	PIXEndEvent(cmdList.Get());
}