#include "pch.h"
#include "DX12GraphicsModuleForXaml.h"
#include <Netcode/Graphics/DX12/DX12Common.h>

namespace Netcode::Graphics::DX12 {

	void DX12GraphicsModuleForXaml::CreateSwapChain() {
		swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
		swapChainDesc.Width = 640;
		swapChainDesc.Height = 480;
		swapChainDesc.Format = renderTargetFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = backbufferDepth;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = swapChainFlags;

		com_ptr<IDXGISwapChain1> tempSwapChain;

		DX_API("Failed to create swap chain for composition")
			dxgiFactory->CreateSwapChainForComposition(commandQueue.Get(), &swapChainDesc, nullptr, &tempSwapChain);

		DX_API("Failed to cast swap chain")
			tempSwapChain.As(&swapChain);

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
