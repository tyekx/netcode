#include "DX12GraphicsModule.h"

namespace Egg::Graphics::DX12 {

	void DX12GraphicsModule::Prepare() {
		FrameResource & fr = frameResources.at(backbufferIndex);

		fr.WaitForCompletion();

		DX_API("Failed to reset command allocator")
			fr.commandAllocator->Reset();

		DX_API("Failed to reset command list")
			fr.commandList->Reset(fr.commandAllocator.Get(), nullptr);

		fr.commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(fr.swapChainBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


		/*
		record upload commands first
		@TODO: rethink this, this could kill performance
		*/
		fr.resourceUploader->Prepare();
		textureLibrary.UploadResources(fr.resourceUploader.get());
		geomManager.UploadResources(fr.resourceUploader.get());
		fr.resourceUploader->Process(commandQueue.Get(), copyCommandQueue.Get());
	}

	void DX12GraphicsModule::SetRenderTarget() {
		SetRenderTarget(0);
	}

	void DX12GraphicsModule::SetRenderTarget(HRENDERTARGET rt) {
		FrameResource & fr = frameResources.at(backbufferIndex);
		ID3D12GraphicsCommandList * gcl = fr.commandList.Get();

		if(rt == 0) {
			gcl->OMSetRenderTargets(1, &fr.rtvHandle, FALSE, &fr.dsvHandle);
			gcl->RSSetScissorRects(1, &fr.scissorRect);
			gcl->RSSetViewports(1, &fr.viewPort);
			gcl->ClearRenderTargetView(fr.rtvHandle, rtvClearValue.Color, 0, nullptr);
			gcl->ClearDepthStencilView(fr.dsvHandle, D3D12_CLEAR_FLAG_DEPTH, dsvClearValue.DepthStencil.Depth, dsvClearValue.DepthStencil.Stencil, 0, nullptr);
		}
	}

	void DX12GraphicsModule::ClearRenderTarget() {
		// @TODO
	}

	void DX12GraphicsModule::Record(HITEM item) {
		RenderItem * renderItem = renderItemColl.GetItem(item);
		renderItemBuffer.push_back(renderItem);
	}

	void DX12GraphicsModule::Render() {
		FrameResource & fr = frameResources.at(backbufferIndex);
		ID3D12GraphicsCommandList * gcl = fr.commandList.Get();

		textureLibrary.SetDescriptorHeap(gcl);

		for(RenderItem * i : renderItemBuffer) {
			i->Render(gcl);
		}

		renderItemBuffer.clear();

		fr.FinishRecording();

		ID3D12CommandList * cls[] = { fr.commandList.Get() };

		commandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);

		commandQueue->Signal(fr.fence.Get(), fr.fenceValue);
	}

	void DX12GraphicsModule::Present() {
		DX_API("Failed to present swap chain")
			swapChain->Present(0, 0);

		NextBackBufferIndex();
	}

	void DX12GraphicsModule::Start(Module::AApp * app)  {
		hwnd = reinterpret_cast<HWND>(app->window->GetUnderlyingPointer());
		backbufferDepth = 2;
		depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

		DX_API("Failed to create debug layer")
			D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));

		if(IsDebuggerPresent()) {
			debugController->EnableDebugLayer();
		}

		// triple buffering is the max allowed
		frameResources.reserve(3);

		rtvClearValue.Color[0] = 0.0f;
		rtvClearValue.Color[1] = 0.2f;
		rtvClearValue.Color[2] = 0.4f;
		rtvClearValue.Color[3] = 1.0f;
		rtvClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		CreateFactory();

		QueryAdapters();

		QuerySyncSupport();

		CreateDevice();

		CreateCommandQueue();

		CreateSwapChain();

		CreateSwapChainResources();

		textureLibrary.CreateResources(device.Get());

		geomManager.CreateResources(device.Get());

		matManager.CreateResources(device.Get());

		cbufferAllocator.CreateResources(device.Get());

		renderItemBuffer.reserve(1024);
	}

}
