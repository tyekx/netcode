#include "DX12GraphicsModule.h"
#include "DX12SpriteFont.h"

namespace Egg::Graphics::DX12 {
	void DX12GraphicsModule::NextBackBufferIndex() {
		backbufferIndex = (backbufferIndex + 1) % backbufferDepth;
	}

	void DX12GraphicsModule::CreateFactory() {
		DX_API("Failed to create dxgi factory")
			CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
	}

	void DX12GraphicsModule::CreateDevice() {
		com_ptr<ID3D12Device5> tempDevice;

		// always the 0th index will be tried first for creation, then we upgrade it as high as possible
		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_12_1,
		};

		D3D12_FEATURE_DATA_FEATURE_LEVELS queryDataFeatureLevels;
		queryDataFeatureLevels.MaxSupportedFeatureLevel = featureLevels[0];
		queryDataFeatureLevels.NumFeatureLevels = _countof(featureLevels);
		queryDataFeatureLevels.pFeatureLevelsRequested = featureLevels;

		DX_API("Failed to create device with %s", FeatureLevelToString(queryDataFeatureLevels.MaxSupportedFeatureLevel))
			D3D12CreateDevice(nullptr, queryDataFeatureLevels.MaxSupportedFeatureLevel, IID_PPV_ARGS(tempDevice.GetAddressOf()));

		DX_API("Failed to query supported feature levels")
			tempDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &queryDataFeatureLevels, sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS));

		tempDevice.Reset();

		DX_API("Failed to upgrade device to %s", FeatureLevelToString(queryDataFeatureLevels.MaxSupportedFeatureLevel))
			D3D12CreateDevice(nullptr, queryDataFeatureLevels.MaxSupportedFeatureLevel, IID_PPV_ARGS(tempDevice.GetAddressOf()));

		Egg::Utility::Debugf("Created DX12 device with %s\r\n", FeatureLevelToString(queryDataFeatureLevels.MaxSupportedFeatureLevel));

		device = std::move(tempDevice);
	}

	void DX12GraphicsModule::ClearAdapters() {
		for(UINT i = 0; i < adaptersLength; ++i) {
			adapters[i].Reset();
		}
		adaptersLength = 0;
	}
	
	void DX12GraphicsModule::QueryAdapters() {
		if(adaptersLength != 0) {
			ClearAdapters();
		}

		com_ptr<IDXGIAdapter1> tempAdapter;

		for(UINT i = 0; dxgiFactory->EnumAdapters1(i, tempAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i) {
			DX_API("Failed to cast IDXGIAdapter1 to IDXGIAdapter3")
				tempAdapter.As(&adapters[adaptersLength]);

			DXGI_ADAPTER_DESC2 adapterDesc;

			DX_API("Failed to query adapter desc")
				adapters[adaptersLength]->GetDesc2(&adapterDesc);

			Egg::Utility::Debugf("Graphics adapter: %S\r\n", adapterDesc.Description);

			// not calling reset here results in memory leak
			tempAdapter.Reset();

			++adaptersLength;
		}
	}
	
	void DX12GraphicsModule::CreateCommandQueue() {
		D3D12_COMMAND_QUEUE_DESC cqd;
		cqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		cqd.NodeMask = 0;
		cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		cqd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		DX_API("Failed to create direct command queue")
			device->CreateCommandQueue(&cqd, IID_PPV_ARGS(commandQueue.GetAddressOf()));

		D3D12_COMMAND_QUEUE_DESC copyCqd;
		copyCqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		copyCqd.NodeMask = 0;
		copyCqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		copyCqd.Type = D3D12_COMMAND_LIST_TYPE_COPY;

		DX_API("Failed to create copy command queue")
			device->CreateCommandQueue(&copyCqd, IID_PPV_ARGS(copyCommandQueue.GetAddressOf()));
	}
	
	void DX12GraphicsModule::QuerySyncSupport() {
		BOOL allowTearing;

		DX_API("Failed to query dxgi feature support: allow tearing")
			dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

		swapChainFlags = (allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	}
	
	void DX12GraphicsModule::CreateSwapChain() {
		swapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
		// if you specify width/height as 0, the CreateSwapChainForHwnd will query it from the output window
		swapChainDesc.Width = 0;
		swapChainDesc.Height = 0;
		swapChainDesc.Format = rtvClearValue.Format;
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

		DXGI_SWAP_CHAIN_DESC1 scDesc;

		DX_API("failed to get swap chain desc")
			swapChain->GetDesc1(&scDesc);

		width = scDesc.Width;
		height = scDesc.Height;
	}
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

		TestFont(0);

		fr.FinishRecording();

		ID3D12CommandList * cls[] = { fr.commandList.Get() };

		commandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);

	}

	void DX12GraphicsModule::Present() {
		FrameResource & fr = frameResources.at(backbufferIndex);

		DX_API("Failed to present swap chain")
			swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);

		commandQueue->Signal(fr.fence.Get(), fr.fenceValue);

		presentedBackbufferIndex = backbufferIndex;
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

		spriteBatch = std::make_unique<SpriteBatch>(device.Get(), frameResources[backbufferIndex].resourceUploader.get(), SpriteBatchPipelineStateDescription(), &cbufferAllocator, &frameResources[backbufferIndex].viewPort);

		fontLibrary.CreateResources(device.Get(), &textureLibrary);

		renderItemBuffer.reserve(1024);
	}

	void DX12GraphicsModule::Shutdown() {

	}

	void DX12GraphicsModule::OnResized(int newWidth, int newHeight) {
		UINT w = static_cast<UINT>(newWidth);
		UINT h = static_cast<UINT>(newHeight);

		if(width != w || height != h) {
			width = w;
			height = h;
			ReleaseSwapChainResources();

			DX_API("Failed to resize buffers")
				swapChain->ResizeBuffers(backbufferDepth, width, height, DXGI_FORMAT_UNKNOWN, swapChainFlags);

			CreateSwapChainResources();
		}
	}

	float DX12GraphicsModule::GetAspectRatio() const
	{
		return static_cast<float>(width) / static_cast<float>(height);
	}
	
	void DX12GraphicsModule::ReleaseSwapChainResources() {
		Log::Debug("Releasing Swap Chain resources");

		if(presentedBackbufferIndex < frameResources.size()) {
			Log::Debug("Waiting for completion");
			frameResources[presentedBackbufferIndex].WaitForCompletion();
			DX_API("Failed to signal from command queue")
				commandQueue->Signal(frameResources[presentedBackbufferIndex].fence.Get(), frameResources[presentedBackbufferIndex].fenceValue);
		}

		for(UINT i = 0; i < frameResources.size(); ++i) {
			frameResources[i].swapChainBuffer.Reset();
			frameResources[i].dsvHandle.ptr = 0;
			frameResources[i].rtvHandle.ptr = 0;
		}

		dsvResource.Reset();
		rtvDescHeap.Reset();
		dsvDescHeap.Reset();
		presentedBackbufferIndex = UINT_MAX;
	}
	
	void DX12GraphicsModule::CreateSwapChainResources() {
		DXGI_SWAP_CHAIN_DESC1 scDesc;

		DX_API("failed to get swap chain desc")
			swapChain->GetDesc1(&scDesc);

		backbufferDepth = scDesc.BufferCount;

		D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc;
		rtvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvDescHeapDesc.NodeMask = 0;
		rtvDescHeapDesc.NumDescriptors = backbufferDepth;
		rtvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		D3D12_DESCRIPTOR_HEAP_DESC dsvDescHeapDesc;
		dsvDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvDescHeapDesc.NodeMask = 0;
		dsvDescHeapDesc.NumDescriptors = 1;
		dsvDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		DX_API("Failed to create srv descriptor heap")
			device->CreateDescriptorHeap(&rtvDescHeapDesc, IID_PPV_ARGS(rtvDescHeap.GetAddressOf()));

		DX_API("Failed to create dsv descriptor heap")
			device->CreateDescriptorHeap(&dsvDescHeapDesc, IID_PPV_ARGS(dsvDescHeap.GetAddressOf()));

		while(frameResources.size() < backbufferDepth) {
			frameResources.emplace_back();
			frameResources.back().CreateResources(device.Get());
		}

		dsvClearValue.DepthStencil.Depth = 1.0f;
		dsvClearValue.DepthStencil.Stencil = 0;
		dsvClearValue.Format = depthStencilFormat;

		DX_API("Failed to create dsv resource")
			device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
											D3D12_HEAP_FLAG_NONE,
											&CD3DX12_RESOURCE_DESC::Tex2D(depthStencilFormat, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
											D3D12_RESOURCE_STATE_DEPTH_WRITE,
											&dsvClearValue,
											IID_PPV_ARGS(dsvResource.GetAddressOf()));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvd = { };
		dsvd.Format = depthStencilFormat;
		dsvd.Flags = D3D12_DSV_FLAG_NONE;
		dsvd.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		device->CreateDepthStencilView(dsvResource.Get(), &dsvd, dsvDescHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_VIEWPORT viewPort;
		viewPort.Height = static_cast<float>(height);
		viewPort.Width = static_cast<float>(width);
		viewPort.TopLeftX = 0.0f;
		viewPort.TopLeftY = 0.0f;
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;

		aspectRatio = viewPort.Width / viewPort.Height;

		D3D12_RECT scissorRect;
		scissorRect.top = 0;
		scissorRect.left = 0;
		scissorRect.bottom = height;
		scissorRect.right = width;

		rtvDescHeapIncrement = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for(UINT i = 0; i < backbufferDepth; ++i) {
			DX_API("Failed to get swap chain buffer")
				swapChain->GetBuffer(i, IID_PPV_ARGS(frameResources[i].swapChainBuffer.GetAddressOf()));

			D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
			cpuDescHandle.ptr += i * rtvDescHeapIncrement;

			device->CreateRenderTargetView(frameResources[i].swapChainBuffer.Get(), nullptr, cpuDescHandle);

			frameResources[i].rtvHandle = cpuDescHandle;
			frameResources[i].dsvHandle = dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
			frameResources[i].viewPort = viewPort;
			frameResources[i].scissorRect = scissorRect;
			frameResources[i].dsvClearValue = dsvClearValue;
			frameResources[i].rtvClearValue = rtvClearValue;
		}

		backbufferIndex = swapChain->GetCurrentBackBufferIndex();
	}

	RenderItem * itm;

	HFONT DX12GraphicsModule::LoadFont(const std::wstring & fontName) {
		FrameResource & fr = frameResources.at(backbufferIndex);

		HTEXTURE tex = textureLibrary.LoadTexture2D(L"btn_background.png");

		HTEXTURE tex2 = textureLibrary.LoadTexture2D(L"debug.png");

		itm = renderItemColl.GetItem(renderItemColl.CreateItem());
		
		textureLibrary.AllocateTextures(itm, 1);

		textureLibrary.SetTexture(itm, 0, tex);

		return fontLibrary.LoadFont(fontName, fr.resourceUploader.get());
	}

	void DX12GraphicsModule::TestFont(HFONT font) {
		FrameResource & fr = frameResources.at(backbufferIndex);
		spriteBatch->Begin(fr.commandList.Get());


		SpriteFont * f = fontLibrary.Get(font);
		f->DrawString(spriteBatch.get(), "Hello World", DirectX::g_XMZero);

		spriteBatch->Draw(itm->texturesHandle, DirectX::XMUINT2(512, 128), DirectX::XMFLOAT2{ 100.0f, 300.0f });

		//spriteBatch->Draw(f->texture, f->textureSize, DirectX::XMFLOAT2{ 0, 100.0f });

		spriteBatch->End();
	}

}
