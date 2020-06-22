#include "DX12GraphicsModule.h"
#include "DX12Builders.h"
#include "DX12Platform.h"
#include "DX12FrameGraphExecutor.h"
#include <sstream>

#include "../../Config.h"

namespace Netcode::Graphics::DX12 {
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

			Log::Info("Graphics adapter: {0}", Netcode::Utility::ToNarrowString(adapterDesc.Description));

			Log::Info("    Adapter dedicated VRAM: {0}", adapterDesc.DedicatedVideoMemory);
			Log::Info("        Adapter system RAM: {0}", adapterDesc.DedicatedSystemMemory);
			Log::Info("        Adapter shared RAM: {0}", adapterDesc.SharedSystemMemory);

			// not calling reset here results in memory leak
			tempAdapter.Reset();

			++adaptersLength;
		}
	}
	
	void DX12GraphicsModule::CreateCommandQueue() {
		{
			D3D12_COMMAND_QUEUE_DESC cqd;
			cqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			cqd.NodeMask = 0;
			cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			cqd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

			DX_API("Failed to create direct command queue")
				device->CreateCommandQueue(&cqd, IID_PPV_ARGS(commandQueue.GetAddressOf()));
		}

		{
			D3D12_COMMAND_QUEUE_DESC computeCqd;
			computeCqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			computeCqd.NodeMask = 0;
			computeCqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			computeCqd.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;

			DX_API("Failed to create compute command queue")
				device->CreateCommandQueue(&computeCqd, IID_PPV_ARGS(computeCommandQueue.GetAddressOf()));
		}

		{
			D3D12_COMMAND_QUEUE_DESC copyCqd;
			copyCqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			copyCqd.NodeMask = 0;
			copyCqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			copyCqd.Type = D3D12_COMMAND_LIST_TYPE_COPY;

			DX_API("Failed to create copy command queue")
				device->CreateCommandQueue(&copyCqd, IID_PPV_ARGS(copyCommandQueue.GetAddressOf()));
		}
	}

	void DX12GraphicsModule::CreateFences()
	{
		mainFence = stackAllocator.MakeShared<DX12Fence>(device.Get(), 0);
		//std::make_shared<DX12Fence>(device.Get(), 0);
		uploadFence = stackAllocator.MakeShared<DX12Fence>(device.Get(), 0);
		//std::make_shared<DX12Fence>(device.Get(), 0);
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
		shaderLibrary = stackAllocator.MakeShared<DX12ShaderLibrary>();
		//std::make_shared<DX12ShaderLibrary>();

		rootSigLibrary = stackAllocator.MakeShared<DX12RootSignatureLibrary>(objectAllocator, device);

		streamOutputLibrary = stackAllocator.MakeShared<DX12StreamOutputLibrary>(objectAllocator);

		inputLayoutLibrary = stackAllocator.MakeShared<DX12InputLayoutLibrary>(objectAllocator);

		gPipelineLibrary = stackAllocator.MakeShared<DX12GPipelineStateLibrary>(objectAllocator, device);

		cPipelineLibrary = stackAllocator.MakeShared<DX12CPipelineStateLibrary>(objectAllocator, device);

		spriteFontLibrary = stackAllocator.MakeShared<DX12SpriteFontLibrary>(objectAllocator);
		spriteFontLibrary->frameCtx = frame;
		spriteFontLibrary->resourceCtx = resources;
	}

	void DX12GraphicsModule::SetContextReferences()
	{
		heapManager = stackAllocator.MakeShared<DX12HeapManager>();
		heapManager->SetDevice(device);

		resourcePool.SetHeapManager(heapManager);

		resourceContext->descHeaps = &dheaps;
		resourceContext->SetResourcePool(&resourcePool);
		resourceContext->SetDevice(device);
		resourceContext->backbufferExtents = scissorRect;

		cbufferPool.SetHeapManager(heapManager);
		dheaps.CreateResources(device);
	}

	void DX12GraphicsModule::CreateContexts() {
		resourceContext = stackAllocator.MakeShared<DX12ResourceContext>();
		Netcode::Module::IGraphicsModule::resources = resourceContext.get();
		Netcode::Module::IGraphicsModule::frame = this;
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
		dheaps.Prepare();

		resourceContext->backbufferExtents = scissorRect;
	}

	void DX12GraphicsModule::Present() {
		DX_API("Failed to present swap chain")
			swapChain->Present(0, (displayMode != DisplayMode::FULLSCREEN) ? DXGI_PRESENT_ALLOW_TEARING : 0);

		mainFence->Increment();

		DX_API("Failed to signal fence")
			commandQueue->Signal(mainFence->GetFence(), mainFence->GetValue());

		presentedBackbufferIndex = backbufferIndex;
	}

	void DX12GraphicsModule::Start(Module::AApp * app)  {
		objectAllocator.SetDefaultAlignment(8);
		objectAllocator.SetDefaultPageSize(1<<18);
		objectAllocator.ReserveFirstPage();

		eventSystem = app->events.get();
		if(app->window != nullptr) {
			hwnd = reinterpret_cast<HWND>(app->window->GetUnderlyingPointer());
		}
		backbufferDepth = 2;
		depthStencilFormat = Config::Get<DXGI_FORMAT>("graphics.depthStencil:Format");;
		Float4 cColor = Config::Get<Float4>("graphics.clearColor:Float4");
		clearColor.r = cColor.x;
		clearColor.g = cColor.y;
		clearColor.b = cColor.z;
		clearColor.a = cColor.w;

		DX_API("Failed to create debug layer")
			D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));

		const bool debugEnabled = Config::Get<bool>("graphics.debug.enabled:bool");
		if(IsDebuggerPresent() && debugEnabled) {
			debugController->EnableDebugLayer();

			if(Config::Get<bool>("graphics.debug.gpuBasedValidation:bool")) {
				debugController->SetEnableGPUBasedValidation(TRUE);
			}
		}

		// triple buffering is the max allowed
		frameResources.reserve(3);

		renderTargetFormat = Config::Get<DXGI_FORMAT>("graphics.backbuffer:Format");

		CreateFactory();

		QueryAdapters();

		QuerySyncSupport();

		CreateDevice();

		CreateCommandQueue();

		CreateFences();

		commandListPool = stackAllocator.MakeShared<DX12CommandListPool>(device);

		CreateContexts();

		CreateSwapChain();

		SetContextReferences();

		renderTargetViews = std::dynamic_pointer_cast<DX12ResourceViews>(resources->CreateRenderTargetViews(3));
		depthStencilView = std::dynamic_pointer_cast<DX12ResourceViews>(resources->CreateDepthStencilView());

		CreateLibraries();

		CreateSwapChainResources();

#if defined(NETCODE_DEBUG)
		if(debugEnabled) {
			debugContext = objectAllocator.MakeShared<DX12DebugContext>();
			debugContext->CreateResources(this);
			debug = debugContext.get();
		}
#endif
	}

	void DX12GraphicsModule::DeviceSync()
	{
		// command lists can be reset while they are being executed
		for(auto & i : inFlightCommandLists) {
			i.ResetCommandList();
		}

		mainFence->HostWait();

		inFlightCommandLists.clear();
	}

	void DX12GraphicsModule::CompleteFrame()
	{
		NextBackBufferIndex();

		cbufferPool.Clear();
		dheaps.Reset();
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

		Netcode::Module::AppEvent evt;
		evt.resizeArgs.x = w;
		evt.resizeArgs.y = h;
		evt.type = Netcode::Module::EAppEventType::RESIZED;

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
		com_ptr<ID3D12Resource> uploadResource;

		CommandList directCl = commandListPool->GetDirect();

		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		barriers.reserve(upload.BarrierTasks().size());

		for(const auto & barrier : upload.BarrierTasks()) {
			ID3D12Resource * resouce = std::dynamic_pointer_cast<DX12Resource>(barrier.resourceHandle)->resource.Get();
			barriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resouce, GetNativeState(barrier.before), GetNativeState(barrier.after)));
		}

		size_t totalSize = 0;

		for(const auto & task : upload.UploadTasks()) {
			if(task.type == UploadTaskType::BUFFER) {
				totalSize += Utility::Align64K<size_t>(task.bufferTask.srcDataSizeInBytes);
			} else if(task.type == UploadTaskType::TEXTURE) {
				TextureRef tex = task.textureTask.texture;
				uint16_t imgCount = tex->GetImageCount();

				size_t sum = 0;
				for(uint16_t imgI = 0; imgI < imgCount; ++imgI) {
					const Image* imgR = tex->GetImage(0, imgI, 0);
					sum += imgR->slicePitch;
				}
				totalSize += Utility::Align64K<size_t>(sum);
			}
		}

		if(totalSize > 0) {

			auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(totalSize);

			DX_API("Failed to create resource")
				device->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&bufferDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(uploadResource.GetAddressOf()));

			size_t offset = 0;


			for(const auto & task : upload.UploadTasks()) {
				if(task.type == UploadTaskType::BUFFER) {
					DX12ResourceRef resource = std::dynamic_pointer_cast<DX12Resource>(task.bufferTask.resourceHandle);

					if(resource->GetDesc().dimension == ResourceDimension::BUFFER) {
						uint8_t * mappedPtr;
						CD3DX12_RANGE nullRange{ 0,0 };

						DX_API("Failed to map upload resource")
							uploadResource->Map(0, &nullRange, reinterpret_cast<void **>(&mappedPtr));

						memcpy(mappedPtr + offset, task.bufferTask.srcData, task.bufferTask.srcDataSizeInBytes);

						uploadResource->Unmap(0, &nullRange);

						directCl.GetCommandList()->CopyBufferRegion(resource->resource.Get(), task.bufferTask.dstDataOffsetInBytes, uploadResource.Get(), offset, task.bufferTask.srcDataSizeInBytes);
					} else {
						D3D12_SUBRESOURCE_DATA data;
						data.RowPitch = (resource->desc.dimension == ResourceDimension::BUFFER) ? task.bufferTask.srcDataSizeInBytes : (resource->desc.strideInBytes * resource->desc.width);
						data.SlicePitch = task.bufferTask.srcDataSizeInBytes;
						data.pData = task.bufferTask.srcData;
						UpdateSubresources(directCl.GetCommandList(), resource->resource.Get(), uploadResource.Get(), offset, 0u, 1u, &data);
					}

					offset += Utility::Align64K<size_t>(task.bufferTask.srcDataSizeInBytes);
				}

				if(task.type == UploadTaskType::TEXTURE) {
					DX12ResourceRef resource = std::dynamic_pointer_cast<DX12Resource>(task.textureTask.resourceHandle);

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

					UpdateSubresources(directCl.GetCommandList(), resource->resource.Get(), uploadResource.Get(), offset, 0u, imgCount, subResData.get());

					offset += Utility::Align64K<size_t>(sum);
				}
			}
		}

		if(!barriers.empty()) {
			directCl.GetCommandList()->ResourceBarrier(static_cast<uint32_t>(barriers.size()), barriers.data());
		}

		DX_API("Failed to close command list")
			directCl.GetCommandList()->Close();

		ID3D12CommandList * cls[] = { directCl.GetCommandList() };

		commandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);
		
		uploadFence->Signal(commandQueue.Get());

		uploadFence->HostWait();
	}

	void DX12GraphicsModule::CullFrameGraph(FrameGraphRef frameGraph)
	{
		std::vector<RenderPassRef> cullable = frameGraph->QueryDanglingRenderPasses();

		while(!cullable.empty()) {
			frameGraph->EraseRenderPasses(std::move(cullable));
			cullable = frameGraph->QueryDanglingRenderPasses();
		}
	}

	void DX12GraphicsModule::ExecuteFrameGraph(FrameGraphRef frameGraph)
	{
		FrameGraphExecutor executor{
			commandListPool.get(),
			heapManager.get(),
			&resourcePool,
			&dheaps,
			&cbufferPool,
			commandQueue.Get(),
			computeCommandQueue.Get(),
			frameResources[backbufferIndex].swapChainBuffer.Get(),
			&inFlightCommandLists,
			&(clearColor.r),
			mainFence,
			renderTargetViews->GetCpuVisibleCpuHandle(backbufferIndex),
			depthStencilView->GetCpuVisibleCpuHandle(0),
			viewport,
			scissorRect
		};

		executor.Execute(frameGraph);
	}

	void DX12GraphicsModule::Run(FrameGraphRef frameGraph, FrameGraphCullMode cullMode)
	{
		if(cullMode == FrameGraphCullMode::ANY) {
			CullFrameGraph(frameGraph);
		}
		ExecuteFrameGraph(frameGraph);

		objectAllocator.Defragment(16);
	}

	Netcode::UInt2 DX12GraphicsModule::GetBackbufferSize() const {
		return Netcode::UInt2{ width, height };
	}

	DXGI_FORMAT DX12GraphicsModule::GetBackbufferFormat() const {
		return renderTargetFormat;
	}

	DXGI_FORMAT DX12GraphicsModule::GetDepthStencilFormat() const {
		return depthStencilFormat;
	}

	void DX12GraphicsModule::ReleaseSwapChainResources() {
		Log::Debug("Releasing Swap Chain resources");

		for(UINT i = 0; i < frameResources.size(); ++i) {
			frameResources[i].swapChainBuffer.Reset();
		}

		depthStencil.reset();

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
		resourceContext->backbufferExtents = scissorRect;

		depthStencil.reset();
		depthStencil = resources->CreateDepthStencil(depthStencilFormat, ResourceType::PERMANENT_DEFAULT);
		depthStencilView->CreateDSV(depthStencil);

		resources->SetDebugName(depthStencil, L"Default Depth Stencil");

		aspectRatio = viewport.Width / viewport.Height;

		for(UINT i = 0; i < backbufferDepth; ++i) {
			DX_API("Failed to get swap chain buffer")
				swapChain->GetBuffer(i, IID_PPV_ARGS(frameResources[i].swapChainBuffer.GetAddressOf()));

			std::wstringstream wss;
			wss << "SwapChainBuffer: " << i;

			DX_API("Failed to set debug name")
				frameResources[i].swapChainBuffer->SetName(wss.str().c_str());

			renderTargetViews->CreateRTV(i, frameResources[i].swapChainBuffer.Get(), renderTargetFormat);
		}

		backbufferIndex = swapChain->GetCurrentBackBufferIndex();
	}

	FenceRef DX12GraphicsModule::CreateFence(uint64_t initialValue)
	{
		return objectAllocator.MakeShared<DX12Fence>(device.Get(), initialValue);
	}
	
	ShaderBuilderRef DX12GraphicsModule::CreateShaderBuilder() {
		return objectAllocator.MakeShared<DX12ShaderBuilder>(shaderLibrary);
	}

	GPipelineStateBuilderRef DX12GraphicsModule::CreateGPipelineStateBuilder() {
		return objectAllocator.MakeShared<DX12GPipelineStateBuilder>(gPipelineLibrary);
	}

	CPipelineStateBuilderRef DX12GraphicsModule::CreateCPipelineStateBuilder()
	{
		return objectAllocator.MakeShared<DX12CPipelineStateBuilder>(cPipelineLibrary);
	}

	InputLayoutBuilderRef DX12GraphicsModule::CreateInputLayoutBuilder() {
		return objectAllocator.MakeShared<DX12InputLayoutBuilder>(objectAllocator, inputLayoutLibrary);
	}

	StreamOutputBuilderRef DX12GraphicsModule::CreateStreamOutputBuilder() {
		return objectAllocator.MakeShared<DX12StreamOutputBuilder>(objectAllocator, streamOutputLibrary);
	}

	RootSignatureBuilderRef DX12GraphicsModule::CreateRootSignatureBuilder() {
		return objectAllocator.MakeShared<DX12RootSignatureBuilder>(rootSigLibrary);
	}

	SpriteFontBuilderRef DX12GraphicsModule::CreateSpriteFontBuilder() {
		return objectAllocator.MakeShared<DX12SpriteFontBuilder>(spriteFontLibrary);
	}

	SpriteBatchBuilderRef DX12GraphicsModule::CreateSpriteBatchBuilder()
	{
		return objectAllocator.MakeShared<DX12SpriteBatchBuilder>(this);
	}

	TextureBuilderRef DX12GraphicsModule::CreateTextureBuilder() {
		return objectAllocator.MakeShared<DX12TextureBuilder>();
	}

	FrameGraphBuilderRef DX12GraphicsModule::CreateFrameGraphBuilder()
	{
		return objectAllocator.MakeShared<DX12FrameGraphBuilder>(resourceContext);
	}

}
