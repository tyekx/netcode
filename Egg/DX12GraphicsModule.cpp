#include "DX12GraphicsModule.h"
#include "DX12Builders.h"
#include "DX12Platform.h"

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
			tempDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &queryDataFeatureLevels, sizeof(queryDataFeatureLevels));

		tempDevice.Reset();

		DX_API("Failed to upgrade device to %s", FeatureLevelToString(queryDataFeatureLevels.MaxSupportedFeatureLevel))
			D3D12CreateDevice(nullptr, queryDataFeatureLevels.MaxSupportedFeatureLevel, IID_PPV_ARGS(tempDevice.GetAddressOf()));

		device = std::move(tempDevice);

		Log::Info("Created DX12 device with: {0}", FeatureLevelToString(queryDataFeatureLevels.MaxSupportedFeatureLevel));

		D3D12_FEATURE_DATA_SHADER_MODEL queryDataShaderModel = {};
		queryDataShaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_4;

		DX_API("Failed to query shader model")
			device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &queryDataShaderModel, sizeof(queryDataShaderModel));

		Log::Info("Highest supported shader model: {0}", ShaderModelToString(queryDataShaderModel.HighestShaderModel));

		D3D12_FEATURE_DATA_ROOT_SIGNATURE queryRootSigVersion = {};
		queryRootSigVersion.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		DX_API("Failed to query highest supported root signature version")
			device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &queryRootSigVersion, sizeof(queryRootSigVersion));

		Log::Info("Highest supported root signature version: {0}", RootSignatureVersionToString(queryRootSigVersion.HighestVersion));

		Platform::MaxSupportedRootSignatureVersion = queryRootSigVersion.HighestVersion;
		Platform::MaxSupportedShaderModel = queryDataShaderModel.HighestShaderModel;
		Platform::RenderTargetViewIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		Platform::ShaderResourceViewIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		Platform::DepthStencilViewIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		Platform::SamplerIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

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

			Log::Info("Graphics adapter: {0}", Egg::Utility::ToNarrowString(adapterDesc.Description));

			Log::Info("    Adapter dedicated VRAM: {0}", adapterDesc.DedicatedVideoMemory);
			Log::Info("        Adapter system RAM: {0}", adapterDesc.DedicatedSystemMemory);
			Log::Info("        Adapter shared RAM: {0}", adapterDesc.SharedSystemMemory);

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

	void * DX12GraphicsModule::GetSwapChain() const {
		return swapChain.Get();
	}

	void DX12GraphicsModule::CreateSwapChain() { }

	void DX12GraphicsModule::CreateLibraries()
	{
		shaderLibrary = std::make_shared<DX12ShaderLibrary>();

		rootSigLibrary = std::make_shared<DX12RootSignatureLibrary>();
		rootSigLibrary->SetDevice(device);

		streamOutputLibrary = std::make_shared<DX12StreamOutputLibrary>();

		inputLayoutLibrary = std::make_shared<DX12InputLayoutLibrary>();

		gPipelineLibrary = std::make_shared<DX12GPipelineStateLibrary>();
		gPipelineLibrary->SetDevice(device);

		cPipelineLibrary = std::make_shared<DX12CPipelineStateLibrary>();
		cPipelineLibrary->SetDevice(device);

		spriteFontLibrary = std::make_shared<DX12SpriteFontLibrary>();
		spriteFontLibrary->frameCtx = frame;
		spriteFontLibrary->resourceCtx = resources;
	}

	void DX12GraphicsModule::SetContextReferences()
	{
		heapManager.SetDevice(device.Get());

		resourcePool.SetHeapManager(&heapManager);

		resourceContext.descHeaps = &dheaps;
		resourceContext.SetResourcePool(&resourcePool);
		resourceContext.SetDevice(device);
		resourceContext.backbufferExtents = scissorRect;

		cbufferPool.SetHeapManager(&heapManager);

		dheaps.CreateResources(device);

		renderContext.cbuffers = &cbufferPool;
		renderContext.resources = &resourcePool;
		renderContext.descHeaps = &dheaps;
	}

	void DX12GraphicsModule::CreateContexts() {
		Egg::Module::IGraphicsModule::renderer = &renderContext;
		Egg::Module::IGraphicsModule::resources = &resourceContext;
		Egg::Module::IGraphicsModule::frame = this;
	}

	void DX12GraphicsModule::UpdateViewport()
	{
		viewport.Height = static_cast<float>(height);
		viewport.Width = static_cast<float>(width);
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		scissorRect.left = 0;
		scissorRect.right = width;
		scissorRect.top = 0;
		scissorRect.bottom = height;
	}

	void DX12GraphicsModule::Prepare() {

		FrameResource & fr = frameResources.at(backbufferIndex);

		dheaps.Prepare();

		DX_API("Failed to reset command allocator")
			fr.commandAllocator->Reset();

		DX_API("Failed to reset command list")
			fr.commandList->Reset(fr.commandAllocator.Get(), nullptr);

		fr.commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(fr.swapChainBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		resourceContext.backbufferExtents = scissorRect;
		renderContext.gcl = fr.commandList.Get();
		renderContext.backbuffer = renderTargetViews->GetCpuVisibleCpuHandle(backbufferIndex);
		renderContext.backbufferDepth = depthStencilView->GetCpuVisibleCpuHandle(0);
		renderContext.defaultViewport = viewport;
		renderContext.defaultScissorRect = scissorRect;

		renderer->SetRenderTargets(0, 0);
		renderer->SetViewport();
		renderer->SetScissorRect();
		renderer->ClearRenderTarget(0, &(clearColor.r));
		renderer->ClearDepthOnly();
	}

	void DX12GraphicsModule::Render() {
		FrameResource & fr = frameResources.at(backbufferIndex);

		fr.FinishRecording();

		ID3D12CommandList * cls[] = { fr.commandList.Get() };

		commandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);

	}

	void DX12GraphicsModule::Present() {
		FrameResource & fr = frameResources.at(backbufferIndex);

		DX_API("Failed to present swap chain")
			swapChain->Present(0, (displayMode != DisplayMode::FULLSCREEN) ? DXGI_PRESENT_ALLOW_TEARING : 0);

		commandQueue->Signal(fr.fence.Get(), fr.fenceValue);

		presentedBackbufferIndex = backbufferIndex;

		NextBackBufferIndex();

		fr.WaitForCompletion();

		cbufferPool.Clear();
		dheaps.Reset();
		resourcePool.ReleaseTransients();
	}

	void DX12GraphicsModule::Start(Module::AApp * app)  {
		eventSystem = app->events.get();
		if(app->window != nullptr) {
			hwnd = reinterpret_cast<HWND>(app->window->GetUnderlyingPointer());
		}
		backbufferDepth = 2;
		depthStencilFormat = DXGI_FORMAT_D32_FLOAT;
		clearColor.r = 0.1f;
		clearColor.g = 0.2f;
		clearColor.b = 0.4f;
		clearColor.a = 1.0f;

		DX_API("Failed to create debug layer")
			D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));

		if(IsDebuggerPresent()) {
			debugController->EnableDebugLayer();
		}

		// triple buffering is the max allowed
		frameResources.reserve(3);

		renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		CreateFactory();

		QueryAdapters();

		QuerySyncSupport();

		CreateDevice();

		CreateCommandQueue();

		CreateContexts();

		CreateSwapChain();

		SetContextReferences();

		renderTargetViews = std::dynamic_pointer_cast<DX12ResourceViews>(resources->CreateRenderTargetViews(3));
		depthStencilView = std::dynamic_pointer_cast<DX12ResourceViews>(resources->CreateDepthStencilView());

		CreateLibraries();

		CreateSwapChainResources();
	}

	void DX12GraphicsModule::Shutdown() {
		if(displayMode == DisplayMode::FULLSCREEN) {
			DX_API("Failed to set windowed state")
				swapChain->SetFullscreenState(FALSE, nullptr);
		}
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

	void DX12GraphicsModule::OnModeChanged(DisplayMode newMode)
	{
		if(displayMode == newMode) {
			return;
		}

		com_ptr<IDXGIOutput> tempOutput;

		DX_API("Failed to get containing output")
			swapChain->GetContainingOutput(tempOutput.GetAddressOf());

		DXGI_OUTPUT_DESC outputDesc;

		DX_API("failed to get output desc")
			tempOutput->GetDesc(&outputDesc);

		int w = 0;
		int h = 0;

		switch(newMode) {
			case DisplayMode::FULLSCREEN:
			case DisplayMode::BORDERLESS:
			{
				lastWindowedModeSize.x = width;
				lastWindowedModeSize.y = height;
				w = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
				h = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
			}
			break;
			case DisplayMode::WINDOWED:
			{
				w = lastWindowedModeSize.x;
				h = lastWindowedModeSize.y;
			}
			break;
		}

		DXGI_MODE_DESC targetMode;
		targetMode.Format = renderTargetFormat;
		targetMode.Height = h;
		targetMode.Width = w;
		targetMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		targetMode.RefreshRate.Denominator = 0;
		targetMode.RefreshRate.Numerator = 0;
		targetMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

		DXGI_MODE_DESC givenMode;

		DX_API("Failed to query closest matching mode")
			tempOutput->FindClosestMatchingMode(&targetMode, &givenMode, nullptr);

		if(newMode == DisplayMode::WINDOWED) {
			givenMode.Width = w;
			givenMode.Height = h;
		} 

		DX_API("Failed to resize target")
			swapChain->ResizeTarget(&givenMode);

		Egg::Module::AppEvent evt;
		evt.resizeArgs.x = w;
		evt.resizeArgs.y = h;
		evt.type = Egg::Module::EAppEventType::RESIZED;

		eventSystem->Broadcast(evt);

		displayMode = newMode;
	}

	RECT DX12GraphicsModule::GetDisplayRect() const
	{
		com_ptr<IDXGIOutput> tempOutput;

		DX_API("Failed to get containing output")
			swapChain->GetContainingOutput(tempOutput.GetAddressOf());

		DXGI_OUTPUT_DESC desc;
		tempOutput->GetDesc(&desc);

		return desc.DesktopCoordinates;
	}

	float DX12GraphicsModule::GetAspectRatio() const {
		return static_cast<float>(width) / static_cast<float>(height);
	}

	void DX12GraphicsModule::SyncUpload(const UploadBatch & upload)
	{
		com_ptr<ID3D12CommandAllocator> allocator;
		com_ptr<ID3D12GraphicsCommandList> gcl;
		com_ptr<ID3D12Fence> fence;
		com_ptr<ID3D12Resource> uploadResource;
		HANDLE evt = CreateEvent(nullptr, FALSE, FALSE, nullptr);

		DX_API("Failed to create command allocator")
			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(allocator.GetAddressOf()));

		DX_API("Failed to create command list")
			device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(gcl.GetAddressOf()));

		DX_API("Failed to create fence")
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		barriers.reserve(upload.BarrierTasks().size());

		for(const auto & barrier : upload.BarrierTasks()) {
			ID3D12Resource* resouce = resourcePool.GetNativeResource(barrier.resourceHandle).resource;
			barriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resouce, GetNativeState(barrier.before), GetNativeState(barrier.after)));
		}

		size_t totalSize = 0;

		
		for(const auto & task : upload.UploadTasks()) {
			if(task.type == UploadTaskType::BUFFER) {
				totalSize += task.bufferTask.srcDataSizeInBytes;
			} else if(task.type == UploadTaskType::TEXTURE) {
				TextureRef tex = task.textureTask.texture;
				uint16_t imgCount = tex->GetImageCount();

				size_t sum = 0;
				for(uint16_t imgI = 0; imgI < imgCount; ++imgI) {
					const Image* imgR = tex->GetImage(0, imgI, 0);
					sum += imgR->slicePitch;
				}
				totalSize += Utility::Align64K(sum);
			}
		}

		if(totalSize > 0) {
			DX_API("Failed to create resource")
				device->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(totalSize),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(uploadResource.GetAddressOf()));

			const D3D12_RANGE nullRange = CD3DX12_RANGE{ 0, 0 };

			size_t offset = 0;


			for(const auto & task : upload.UploadTasks()) {
				if(task.type == UploadTaskType::BUFFER) {
					const auto & gres = resourcePool.GetNativeResource(task.bufferTask.resourceHandle);

					D3D12_SUBRESOURCE_DATA data;
					data.RowPitch = (gres.desc.dimension == ResourceDimension::BUFFER) ? task.bufferTask.srcDataSizeInBytes : (gres.desc.strideInBytes * gres.desc.width);
					data.SlicePitch = task.bufferTask.srcDataSizeInBytes;
					data.pData = task.bufferTask.srcData;
					UpdateSubresources(gcl.Get(), gres.resource, uploadResource.Get(), offset, 0u, 1u, &data);

					offset += Utility::Align64K(task.bufferTask.srcDataSizeInBytes);
				}

				if(task.type == UploadTaskType::TEXTURE) {
					const auto & gres = resourcePool.GetNativeResource(task.textureTask.resourceHandle);

					TextureRef tex = task.textureTask.texture;
					uint16_t imgCount = tex->GetImageCount();

					size_t sum = 0;
					std::unique_ptr<D3D12_SUBRESOURCE_DATA[]> subResData = std::make_unique<D3D12_SUBRESOURCE_DATA[]>(imgCount);

					for(uint16_t imgI = 0; imgI < imgCount; ++imgI) {
						const Image * imgR = tex->GetImage(0, imgI, 0);
						subResData[imgI].pData = imgR->pixels;
						subResData[imgI].RowPitch = imgR->rowPitch;
						subResData[imgI].SlicePitch = imgR->slicePitch;
						sum += imgR->slicePitch;
					}

					UpdateSubresources(gcl.Get(), gres.resource, uploadResource.Get(), offset, 0u, imgCount, subResData.get());

					offset += Utility::Align64K(sum);
				}
			}
		}

		if(!barriers.empty()) {
			gcl->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
		}

		DX_API("Failed to close command list")
			gcl->Close();

		ID3D12CommandList * cls[] = { gcl.Get() };

		commandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);
		
		DX_API("Failed to signal fence")
			commandQueue->Signal(fence.Get(), 1);


		if(fence->GetCompletedValue() != 1) {
			DX_API("Failed to set event")
				fence->SetEventOnCompletion(1, evt);

			WaitForSingleObject(evt, INFINITE);
		}

		CloseHandle(evt);

	}

	DirectX::XMUINT2 DX12GraphicsModule::GetBackbufferSize() const {
		return DirectX::XMUINT2{ width, height };
	}

	void DX12GraphicsModule::ReleaseSwapChainResources() {
		Log::Debug("Releasing Swap Chain resources");

		for(UINT i = 0; i < frameResources.size(); ++i) {
			frameResources[i].swapChainBuffer.Reset();
		}

		if(depthStencil != 0) {
			resources->ReleaseResource(depthStencil);
			depthStencil = 0;
		}

		presentedBackbufferIndex = UINT_MAX;
	}
	
	void DX12GraphicsModule::CreateSwapChainResources() {
		DXGI_SWAP_CHAIN_DESC1 scDesc;

		DX_API("failed to get swap chain desc")
			swapChain->GetDesc1(&scDesc);

		backbufferDepth = scDesc.BufferCount;

		while(frameResources.size() < backbufferDepth) {
			frameResources.emplace_back();
			frameResources.back().CreateResources(device.Get());
		}

		UpdateViewport();
		resourceContext.backbufferExtents = scissorRect;

		depthStencil = resources->CreateDepthStencil(depthStencilFormat, ResourceType::PERMANENT_DEFAULT);
		depthStencilView->CreateDSV(depthStencil);

		aspectRatio = viewport.Width / viewport.Height;

		for(UINT i = 0; i < backbufferDepth; ++i) {
			DX_API("Failed to get swap chain buffer")
				swapChain->GetBuffer(i, IID_PPV_ARGS(frameResources[i].swapChainBuffer.GetAddressOf()));

			renderTargetViews->CreateRTV(i, frameResources[i].swapChainBuffer.Get(), renderTargetFormat);
		}

		backbufferIndex = swapChain->GetCurrentBackBufferIndex();
	}

	FenceRef DX12GraphicsModule::CreateFence(uint64_t initialValue) const
	{
		return std::make_shared<DX12Fence>(initialValue);
	}
	
	ShaderBuilderRef DX12GraphicsModule::CreateShaderBuilder() const {
		return std::make_shared<DX12ShaderBuilder>(shaderLibrary);
	}

	GPipelineStateBuilderRef DX12GraphicsModule::CreateGPipelineStateBuilder() const {
		return std::make_shared<DX12GPipelineStateBuilder>(gPipelineLibrary);
	}

	CPipelineStateBuilderRef DX12GraphicsModule::CreateCPipelineStateBuilder() const
	{
		return std::make_shared<DX12CPipelineStateBuilder>(cPipelineLibrary);
	}

	InputLayoutBuilderRef DX12GraphicsModule::CreateInputLayoutBuilder() const {
		return std::make_shared<DX12InputLayoutBuilder>(inputLayoutLibrary);
	}

	StreamOutputBuilderRef DX12GraphicsModule::CreateStreamOutputBuilder() const {
		return std::make_shared<DX12StreamOutputBuilder>(streamOutputLibrary);
	}

	RootSignatureBuilderRef DX12GraphicsModule::CreateRootSignatureBuilder() const {
		return std::make_shared<DX12RootSignatureBuilder>(rootSigLibrary);
	}

	SpriteFontBuilderRef DX12GraphicsModule::CreateSpriteFontBuilder() const {
		return std::make_shared<DX12SpriteFontBuilder>(spriteFontLibrary);
	}

	SpriteBatchBuilderRef DX12GraphicsModule::CreateSpriteBatchBuilder() const
	{
		return std::make_shared<DX12SpriteBatchBuilder>(this);
	}

	TextureBuilderRef DX12GraphicsModule::CreateTextureBuilder() const {
		return std::make_shared<DX12TextureBuilder>();
	}

}
