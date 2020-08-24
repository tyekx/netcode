#include "DX12GraphicsModule.h"

#include "DX12Builders.h"
#include "DX12Platform.h"
#include "DX12FrameGraphExecutor.h"

#include "DX12SpriteFontLibrary.h"
#include "DX12ShaderLibrary.h"
#include "DX12RootSignatureLibrary.h"
#include "DX12StreamOutputLibrary.h"
#include "DX12InputLayoutLibrary.h"
#include "DX12GPipelineStateLibrary.h"
#include "DX12CPipelineStateLibrary.h"
#include "DX12TextureLibrary.h"
#include "DX12ResourceContext.h"
#include "DX12DebugContext.h"
#include "DX12UploadBatch.h"
#include "DX12DynamicDescriptorHeap.h"
#include "DX12ConstantBufferPool.h"
#include "DX12ResourcePool.h"
#include "DX12Resource.h"
#include "DX12Fence.h"
#include "DX12CommandListPool.h"
#include "DX12FrameGraph.h"
#include "DX12ResourceViews.h"
#include "DX12ShaderVariant.h"

#include <Netcode/Config.h>
#include <Netcode/Utility.h>

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
		mainFence = stackAllocator.MakeShared<DX12::FenceImpl>(device.Get(), 0);
		uploadFence = stackAllocator.MakeShared<DX12::FenceImpl>(device.Get(), 0);
	}
	
	void DX12GraphicsModule::QuerySyncSupport() {
		BOOL allowTearing;

		DX_API("Failed to query dxgi feature support: allow tearing")
			dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

		swapChainFlags = (allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	}

	void DX12GraphicsModule::ClearImportCache() {
		textureLibrary->ClearCache();
	}

	void * DX12GraphicsModule::GetSwapChain() const {
		return swapChain.Get();
	}

	void DX12GraphicsModule::CreateSwapChain() {
		
	}

	void DX12GraphicsModule::CreateLibraries()
	{
		shaderLibrary = stackAllocator.MakeShared<ShaderLibrary>();

		rootSigLibrary = stackAllocator.MakeShared<RootSignatureLibrary>(objectAllocator, device);

		streamOutputLibrary = stackAllocator.MakeShared<StreamOutputLibrary>(objectAllocator);

		inputLayoutLibrary = stackAllocator.MakeShared<InputLayoutLibrary>(objectAllocator);

		gPipelineLibrary = stackAllocator.MakeShared<GPipelineStateLibrary>(objectAllocator, device);

		cPipelineLibrary = stackAllocator.MakeShared<CPipelineStateLibrary>(objectAllocator, device);

		spriteFontLibrary = stackAllocator.MakeShared<SpriteFontLibrary>(objectAllocator, resources, frame);

		textureLibrary = stackAllocator.MakeShared<TextureLibrary>(objectAllocator, this);
	}

	void DX12GraphicsModule::SetContextReferences()
	{
		resourceContext->descHeaps = dheaps.get();
		resourceContext->SetResourcePool(resourcePool.get());
		resourceContext->SetDevice(device);
		resourceContext->backbufferExtents = scissorRect;
	}

	void DX12GraphicsModule::CreateContexts() {
		resourceContext = stackAllocator.MakeShared<DX12ResourceContext>();
		Netcode::Module::IGraphicsModule::resources = resourceContext.get();
		Netcode::Module::IGraphicsModule::frame = this;

		heapManager = stackAllocator.MakeShared<HeapManager>(device);
		commandListPool = stackAllocator.MakeShared<CommandListPool>(device);
		dheaps = stackAllocator.MakeShared<DynamicDescriptorHeap>(device);
		resourcePool = stackAllocator.MakeShared<ResourcePool>(heapManager);
		cbufferPool = stackAllocator.MakeShared<ConstantBufferPool>(heapManager);
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
		dheaps->Prepare();

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
		depthStencilFormat = Config::Get<DXGI_FORMAT>("graphics.depthStencil:Format");
		Float4 cColor = Config::Get<Float4>("graphics.clearColor:Float4");
		clearColor.r = cColor.x;
		clearColor.g = cColor.y;
		clearColor.b = cColor.z;
		clearColor.a = cColor.w;

		const bool debugEnabled = Config::Get<bool>("graphics.debug.enabled:bool");
		if(IsDebuggerPresent() && debugEnabled) {
			DX_API("Failed to create debug layer")
				D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));

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

		CreateContexts();

		CreateSwapChain();

		SetContextReferences();

		renderTargetViews = std::dynamic_pointer_cast<DX12::ResourceViewsImpl>(resources->CreateRenderTargetViews(3));
		depthStencilView = std::dynamic_pointer_cast<DX12::ResourceViewsImpl>(resources->CreateDepthStencilView());

		CreateLibraries();

		CreateSwapChainResources();

#if defined(NETCODE_DEBUG)
		if(debugEnabled) {
			debugContext = objectAllocator.MakeShared<DebugContext>();
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

		cbufferPool->Clear();
		dheaps->Reset();
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

	Rect DX12GraphicsModule::GetDisplayRect() const
	{
		com_ptr<IDXGIOutput> tempOutput;

		DX_API("Failed to get containing output")
			swapChain->GetContainingOutput(tempOutput.GetAddressOf());

		DXGI_OUTPUT_DESC desc;
		tempOutput->GetDesc(&desc);

		return (*reinterpret_cast<Rect *>(&desc.DesktopCoordinates));
	}

	float DX12GraphicsModule::GetAspectRatio() const {
		return static_cast<float>(width) / static_cast<float>(height);
	}

	void DX12GraphicsModule::SyncUpload(Ref<Netcode::Graphics::UploadBatch> uploadBatch)
	{
		Ref<DX12::UploadBatchImpl> upload = std::dynamic_pointer_cast<DX12::UploadBatchImpl>(uploadBatch);

		com_ptr<ID3D12Resource> uploadResource;

		CommandList directCl = commandListPool->GetDirect();

		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		barriers.reserve(upload->BarrierTasks().size());

		for(const auto & barrier : upload->BarrierTasks()) {
			ID3D12Resource * resouce = std::dynamic_pointer_cast<Resource>(barrier.resourceHandle)->resource.Get();
			barriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resouce, GetNativeState(barrier.before), GetNativeState(barrier.after)));
		}

		size_t totalSize = 0;

		for(const auto & task : upload->UploadTasks()) {
			if(std::holds_alternative<BufferUploadTask>(task)) {
				totalSize += Utility::Align64K<size_t>(std::get<BufferUploadTask>(task).srcDataSizeInBytes);
			}

			if(std::holds_alternative<TextureUploadTask>(task)) {
				Ref<Netcode::Texture> tex = std::get<TextureUploadTask>(task).texture;
				uint16_t imgCount = tex->GetImageCount();
				uint16_t mipCount = tex->GetMipLevelCount();

				size_t sum = 0;
				for(uint16_t mip = 0; mip < mipCount; ++mip) {
					for(uint16_t imgI = 0; imgI < imgCount; ++imgI) {
						const Image * imgR = tex->GetImage(mip, imgI, 0);
						sum += imgR->slicePitch;
					}
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


			for(const auto & t : upload->UploadTasks()) {
				if(std::holds_alternative<BufferUploadTask>(t)) {
					const BufferUploadTask & bufferTask = std::get<BufferUploadTask>(t);

					Ref<DX12::Resource> resource = std::dynamic_pointer_cast<DX12::Resource>(bufferTask.resourceHandle);

					if(resource->GetDesc().dimension == ResourceDimension::BUFFER) {
						uint8_t * mappedPtr;
						CD3DX12_RANGE nullRange{ 0,0 };

						DX_API("Failed to map upload resource")
							uploadResource->Map(0, &nullRange, reinterpret_cast<void **>(&mappedPtr));

						memcpy(mappedPtr + offset, bufferTask.srcData, bufferTask.srcDataSizeInBytes);

						uploadResource->Unmap(0, &nullRange);

						directCl.GetCommandList()->CopyBufferRegion(resource->resource.Get(), bufferTask.dstDataOffsetInBytes, uploadResource.Get(), offset, bufferTask.srcDataSizeInBytes);
					} else {
						D3D12_SUBRESOURCE_DATA data;
						data.RowPitch = (resource->desc.dimension == ResourceDimension::BUFFER) ? bufferTask.srcDataSizeInBytes : (resource->desc.strideInBytes * resource->desc.width);
						data.SlicePitch = bufferTask.srcDataSizeInBytes;
						data.pData = bufferTask.srcData;
						UpdateSubresources(directCl.GetCommandList(), resource->resource.Get(), uploadResource.Get(), offset, 0u, 1u, &data);
					}

					offset += Utility::Align64K<size_t>(bufferTask.srcDataSizeInBytes);
				}

				if(std::holds_alternative<TextureUploadTask>(t)) {
					const TextureUploadTask & textureTask = std::get<TextureUploadTask>(t);

					Ref<DX12::Resource> resource = std::dynamic_pointer_cast<DX12::Resource>(textureTask.resourceHandle);

					Ref<Netcode::Texture> tex = textureTask.texture;
					uint16_t imgCount = tex->GetImageCount();
					uint16_t mipCount = tex->GetMipLevelCount();

					size_t sum = 0;
					std::unique_ptr<D3D12_SUBRESOURCE_DATA[]> subResData = std::make_unique<D3D12_SUBRESOURCE_DATA[]>(mipCount * imgCount);

					uint16_t ii = 0;
					for(uint16_t imgI = 0; imgI < imgCount; ++imgI) {
						for(uint16_t mipI = 0; mipI < mipCount; ++mipI) {
							const Image * imgR = tex->GetImage(mipI, imgI, 0);
							subResData[ii].pData = imgR->pixels;
							subResData[ii].RowPitch = imgR->rowPitch;
							subResData[ii].SlicePitch = imgR->slicePitch;
							ii++;
							sum += imgR->slicePitch;
						}
					}

					UpdateSubresources(directCl.GetCommandList(), resource->resource.Get(), uploadResource.Get(), offset, 0u, ii, subResData.get());

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

	void DX12GraphicsModule::CullFrameGraph(Ptr<Netcode::FrameGraph> frameGraph)
	{
		std::vector<Ref<Netcode::RenderPass>> cullable = frameGraph->QueryDanglingRenderPasses();

		while(!cullable.empty()) {
			frameGraph->EraseRenderPasses(std::move(cullable));
			cullable = frameGraph->QueryDanglingRenderPasses();
		}
	}

	void DX12GraphicsModule::ExecuteFrameGraph(Ref<Netcode::FrameGraph> frameGraph)
	{
		FrameGraphExecutor executor{
			commandListPool.get(),
			heapManager.get(),
			resourcePool.get(),
			dheaps.get(),
			cbufferPool.get(),
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

		executor.Execute(std::move(frameGraph));
	}

	void DX12GraphicsModule::Run(Ref<Netcode::FrameGraph> frameGraph, FrameGraphCullMode cullMode)
	{
		if(cullMode == FrameGraphCullMode::ANY) {
			CullFrameGraph(frameGraph.get());
		}
		ExecuteFrameGraph(std::move(frameGraph));

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
		depthStencilView->CreateDSV(depthStencil.get());

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

	Ref<Fence> DX12GraphicsModule::CreateFence(uint64_t initialValue)
	{
		return objectAllocator.MakeShared<FenceImpl>(device.Get(), initialValue);
	}
	
	Ref<ShaderBuilder> DX12GraphicsModule::CreateShaderBuilder() {
		return objectAllocator.MakeShared<ShaderBuilderImpl>(shaderLibrary);
	}

	Ref<GPipelineStateBuilder> DX12GraphicsModule::CreateGPipelineStateBuilder() {
		return objectAllocator.MakeShared<GPipelineStateBuilderImpl>(gPipelineLibrary);
	}

	Ref<CPipelineStateBuilder> DX12GraphicsModule::CreateCPipelineStateBuilder()
	{
		return objectAllocator.MakeShared<CPipelineStateBuilderImpl>(cPipelineLibrary);
	}

	Ref<InputLayoutBuilder> DX12GraphicsModule::CreateInputLayoutBuilder() {
		return objectAllocator.MakeShared<InputLayoutBuilderImpl>(objectAllocator, inputLayoutLibrary);
	}

	Ref<StreamOutputBuilder> DX12GraphicsModule::CreateStreamOutputBuilder() {
		return objectAllocator.MakeShared<StreamOutputBuilderImpl>(objectAllocator, streamOutputLibrary);
	}

	Ref<RootSignatureBuilder> DX12GraphicsModule::CreateRootSignatureBuilder() {
		return objectAllocator.MakeShared<RootSignatureBuilderImpl>(rootSigLibrary);
	}

	Ref<SpriteFontBuilder> DX12GraphicsModule::CreateSpriteFontBuilder() {
		return objectAllocator.MakeShared<SpriteFontBuilderImpl>(spriteFontLibrary);
	}

	Ref<SpriteBatchBuilder> DX12GraphicsModule::CreateSpriteBatchBuilder()
	{
		return objectAllocator.MakeShared<SpriteBatchBuilderImpl>(this);
	}

	Ref<TextureBuilder> DX12GraphicsModule::CreateTextureBuilder() {
		return objectAllocator.MakeShared<TextureBuilderImpl>(textureLibrary);
	}

	Ref<FrameGraphBuilder> DX12GraphicsModule::CreateFrameGraphBuilder()
	{
		return objectAllocator.MakeShared<FrameGraphBuilderImpl>(resourceContext);
	}

	ID3D12GraphicsCommandList3 * FrameResource::GetCommandList() const {
		return commandList.Get();
	}

	void FrameResource::CreateResources(ID3D12Device * device) {
		DX_API("Failed to create command allocator")
			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.GetAddressOf()));

		DX_API("Failed to create direct command list")
			device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));

		DX_API("Failed to initially close command list")
			commandList->Close();

		fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(fenceEvent == NULL) {
			DX_API("Failed to create windows event") HRESULT_FROM_WIN32(GetLastError());
		}
		fenceValue = 1;

		DX_API("Failed to create fence")
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
	}

	void FrameResource::WaitForCompletion() {
		const UINT64 cv = fenceValue;

		if(fence->GetCompletedValue() < cv) {
			DX_API("Failed to set winapi event")
				fence->SetEventOnCompletion(cv, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
		}

		++fenceValue;
	}

	void FrameResource::FinishRecording() {
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		commandList->ResourceBarrier(1, &barrier);

		DX_API("Failed to close command list")
			commandList->Close();
	}

	void FrameResource::ReleaseResources() {
		commandList.Reset();
		commandAllocator.Reset();
		fence.Reset();
		fenceValue = 0;
	}

}
