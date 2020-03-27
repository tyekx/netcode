#include "DX12GraphicsModuleForWin32.h"

namespace Egg::Graphics::DX12 {

	

	void DX12GraphicsModuleForWin32::Start(Egg::Module::AApp * app) {
		if(app->window != nullptr) {
			hwnd = reinterpret_cast<HWND>(app->window->GetUnderlyingPointer());
		}
		DX12GraphicsModule::Start(app);
	}

	void DX12GraphicsModuleForWin32::CreateSwapChain()
	{
		swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
		// if you specify width/height as 0, the CreateSwapChainForHwnd will query it from the output window
		swapChainDesc.Width = 0;
		swapChainDesc.Height = 0;
		swapChainDesc.Format = renderTargetFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = backbufferDepth;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = swapChainFlags;

		com_ptr<IDXGISwapChain1> tempSwapChain;

		DX_API("Failed to create swap chain for hwnd")
			dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain);

		DX_API("Failed to cast swap chain")
			tempSwapChain.As(&swapChain);

		DX_API("Failed to make window association")
			dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		DX_API("Failed to set background color")
			swapChain->SetBackgroundColor(&clearColor);

		DXGI_SWAP_CHAIN_DESC1 scDesc;

		DX_API("failed to get swap chain desc")
			swapChain->GetDesc1(&scDesc);

		width = scDesc.Width;
		height = scDesc.Height;

		UpdateViewport();
	}

}

