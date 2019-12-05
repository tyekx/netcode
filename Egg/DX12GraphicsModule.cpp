#include "DX12GraphicsModule.h"

namespace Egg::Graphics::DX12 {

	 void DX12GraphicsModule::Start(Module::AApp * app)  {
		hwnd = reinterpret_cast<HWND>(app->window->GetUnderlyingPointer());
		backbufferDepth = 2;
		depthStencilFormat = DXGI_FORMAT_D32_FLOAT;

		DX_API("Failed to create debug layer")
			D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));

		debugController->EnableDebugLayer();

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

		copyResources.CreateResources(device.Get());

		textureLibrary.CreateResources(device.Get());

		geomManager.CreateResources(device.Get());

		matManager.CreateResources(device.Get());

		renderItemBuffer.reserve(1024);
	}

}
