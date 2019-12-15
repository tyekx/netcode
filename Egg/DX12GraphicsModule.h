#pragma once

#include "Utility.h"
#include "Modules.h"
#include <map>

#include "DX12Common.h"
#include "DX12ResourceUploadBatch.h"
#include "DX12Resource.h"
#include "DX12RenderItem.h"
#include "DX12RenderItemAllocator.h"
#include "DX12ShaderManager.h"
#include "DX12TextureLibrary.h"
#include "DX12PipelineStateManager.h"
#include "DX12GeometryManager.h"
#include "DX12MaterialManager.h"
#include "DX12RenderItemCollection.h"
#include "DX12CbufferAllocator.h"

namespace Egg::Graphics::DX12 {

	class FrameResource {
	public:
		com_ptr<ID3D12Resource> swapChainBuffer;
		com_ptr<ID3D12GraphicsCommandList2> commandList;
		com_ptr<ID3D12CommandAllocator> commandAllocator;
		com_ptr<ID3D12Fence1> fence;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
		UINT64 fenceValue;
		HANDLE fenceEvent;
		D3D12_RECT scissorRect;
		D3D12_VIEWPORT viewPort;
		D3D12_CLEAR_VALUE dsvClearValue;
		D3D12_CLEAR_VALUE rtvClearValue;
		std::unique_ptr<Resource::IResourceUploadBatch> resourceUploader;

		ID3D12GraphicsCommandList2 * GetCommandList() const {
			return commandList.Get();
		}

		void CreateResources(ID3D12Device * device) {
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
			fenceValue = 0;

			DX_API("Failed to create fence")
				device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

			resourceUploader = std::make_unique<Resource::ResourceUploadBatch>();
			resourceUploader->CreateResources(device);
		}
		
		void WaitForCompletion() {
			const UINT64 cv = fenceValue;

			if(fence->GetCompletedValue() < cv) {
				DX_API("Failed to set winapi event")
					fence->SetEventOnCompletion(cv, fenceEvent);
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			++fenceValue;
		}

		void FinishRecording() {
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

			DX_API("Failed to close command list")
				commandList->Close();
		}

		void ReleaseResources() {
			commandList.Reset();
			commandAllocator.Reset();
			fence.Reset();
			fenceValue = 0;
		}
	};

	class DX12GraphicsModule : public Egg::Module::IGraphicsModule {
		HWND hwnd;

		com_ptr<IDXGIFactory5> dxgiFactory;

		com_ptr<IDXGISwapChain4> swapChain;
		UINT swapChainFlags;

		com_ptr<ID3D12Device5> device;
		com_ptr<ID3D12CommandQueue> commandQueue;
		com_ptr<ID3D12CommandQueue> copyCommandQueue;
		
		com_ptr<IDXGIAdapter3> adapters[8];
		UINT adaptersLength;

		com_ptr<IDXGIOutput1> outputs[8];
		UINT outputsLength;

		com_ptr<ID3D12DescriptorHeap> rtvDescHeap;
		UINT rtvDescHeapIncrement;
		UINT backbufferDepth;
		UINT width;
		UINT height;

		com_ptr<ID3D12DescriptorHeap> dsvDescHeap;
		com_ptr<ID3D12Resource> dsvResource;
		DXGI_FORMAT depthStencilFormat;
		D3D12_CLEAR_VALUE dsvClearValue;
		D3D12_CLEAR_VALUE rtvClearValue;

		std::vector<FrameResource> frameResources;
		UINT backbufferIndex;
		UINT presentedBackbufferIndex;

		std::vector<DX12::RenderItem*> renderItemBuffer;
		float aspectRatio;

		com_ptr<ID3D12Debug3> debugController;

		void NextBackBufferIndex() {
			backbufferIndex = (backbufferIndex + 1) % backbufferDepth;
		}

		void CreateFactory() {
			DX_API("Failed to create dxgi factory")
				CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
		}

		void CreateDevice() {
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

		void ClearAdapters() {
			for(UINT i = 0; i < adaptersLength; ++i) {
				adapters[i].Reset();
			}
			adaptersLength = 0;
		}

		void QueryAdapters() {
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

				++adaptersLength;
			}
		}

		void CreateCommandQueue() {
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

		void QuerySyncSupport() {
			BOOL allowTearing;

			DX_API("Failed to query dxgi feature support: allow tearing")
				dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

			swapChainFlags = (allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		}

		void CreateSwapChain() {
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

		ShaderManager shaderManager;
		TextureLibrary textureLibrary;
		PipelineStateManager psManager;
		GeometryManager geomManager;
		MaterialManager matManager;
		RenderItemCollection renderItemColl;
		CbufferAllocator cbufferAllocator;

	public:

		virtual void Prepare() override;

		virtual void SetRenderTarget() override;

		virtual void SetRenderTarget(HRENDERTARGET rt) override;

		virtual void ClearRenderTarget() override;

		virtual void Record(HITEM item) override;

		virtual void Render() override;

		virtual void Present() override;

		virtual void Start(Module::AApp * app) override;

		virtual void Shutdown() override { }

		virtual void Resized(int width, int height) override;

		virtual float GetAspectRatio() const override;

		virtual HITEM CreateItem() override {
			return renderItemColl.CreateItem();
		}

		virtual HSHADER LoadShader(const std::wstring & shaderPath) override {
			return shaderManager.LoadShader(shaderPath);
		}

		virtual HINCOMPLETESHADER CreateVertexShader() override {
			return shaderManager.CreateVertexShader();
		}

		virtual HINCOMPLETESHADER CreatePixelShader() override {
			return shaderManager.CreatePixelShader();
		}

		virtual HINCOMPLETESHADER CreateGeometryShader() override {
			return shaderManager.CreateGeometryShader();
		}

		virtual HINCOMPLETESHADER CreateDomainShader() override {
			return shaderManager.CreateDomainShader();
		}

		virtual HINCOMPLETESHADER CreateHullShader() override {
			return shaderManager.CreateHullShader();
		}

		virtual void SetShaderEntry(HINCOMPLETESHADER shader, const std::string & entryFunction) override {
			shaderManager.SetShaderEntry(shader, entryFunction);
		}

		virtual void SetShaderSource(HINCOMPLETESHADER shader, const std::wstring & shaderPath) override {
			shaderManager.SetShaderSource(shader, shaderPath);
		}

		virtual void SetShaderMacros(HINCOMPLETESHADER shader, const std::map<std::string, std::string> & defines) override {
			shaderManager.SetShaderMacros(shader, defines);
		}

		virtual HSHADER CompileShader(HINCOMPLETESHADER shader) override {
			return shaderManager.CompileShader(shader);
		}

		virtual HTEXTURE LoadTexture(const std::wstring & textureMediaPath) override {
			return textureLibrary.LoadTexture2D(textureMediaPath);
		}

		virtual HPSO CreatePipelineState() override {
			return psManager.Create();
		}

		virtual void SetVertexShader(HPSO pso, HSHADER vertexShader) override {
			psManager.SetVertexShader(pso, shaderManager.GetShader(vertexShader));
		}

		virtual void SetPixelShader(HPSO pso, HSHADER pixelShader) override {
			psManager.SetPixelShader(pso, shaderManager.GetShader(pixelShader));
		}

		virtual void SetGeometryShader(HPSO pso, HSHADER geometryShader) override {
			psManager.SetGeometryShader(pso, shaderManager.GetShader(geometryShader));
		}

		virtual void SetHullShader(HPSO pso, HSHADER hullShader) override {
			psManager.SetHullShader(pso, shaderManager.GetShader(hullShader));
		}

		virtual void SetDomainShader(HPSO pso, HSHADER domainShader) override {
			psManager.SetDomainShader(pso, shaderManager.GetShader(domainShader));
		}

		virtual HGEOMETRY CreateGeometry(EGeometryType type = EGeometryType::INDEXED) override {
			return geomManager.CreateGeometry(type);
		}

		virtual void AddVertexBufferLOD(HGEOMETRY geometry, void * ptr, unsigned int sizeInBytes, unsigned int strideInBytes) override {
			geomManager.AddVertexBufferLOD(geometry, ptr, sizeInBytes, strideInBytes);
		}

		virtual void AddIndexBufferLOD(HGEOMETRY geometry, void * ptr, unsigned int sizeInBytes, unsigned int  format) override {
			geomManager.AddIndexBufferLOD(geometry, ptr, sizeInBytes, static_cast<DXGI_FORMAT>(format));
		}

		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int semanticIndex, unsigned int  format, unsigned int byteOffset) override {
			geomManager.AddInputElement(geometry, name, semanticIndex, static_cast<DXGI_FORMAT>(format), byteOffset);
		}

		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int  format, unsigned int byteOffset) override {
			AddInputElement(geometry, name, 0, format, byteOffset);
		}

		virtual HMATERIAL CreateMaterial(HPSO pso, HGEOMETRY geometry) override {
			return matManager.CreateMaterial(psManager.GetGPSO(pso), geomManager.GetInputLayout(geometry));
		}

		virtual void SetMaterial(HITEM item, HMATERIAL material) override {
			matManager.SetRenderItemMaterial(renderItemColl.GetItem(item), material);
		}

		virtual void SetGeometry(HITEM item, HGEOMETRY geometry) override {
			geomManager.SetRenderItemGeometry(renderItemColl.GetItem(item), geometry);
		}


		virtual void AllocateTextures(HITEM item, unsigned int  numTextures) override {
			textureLibrary.AllocateTextures(renderItemColl.GetItem(item), numTextures);
		}

		virtual void SetTexture(HITEM item, unsigned int  slot, HTEXTURE texture) override {
			textureLibrary.SetTexture(renderItemColl.GetItem(item), slot, texture);
		}

		virtual HTEXTURE SetTexture(HITEM item, unsigned int  slot, const std::wstring & texturePath) override {
			HTEXTURE t = textureLibrary.LoadTexture2D(texturePath);
			SetTexture(item, slot, t);
			return t;
		}

		virtual HCBUFFER AllocateCbuffer(unsigned int sizeInBytes) override {
			return cbufferAllocator.AllocateCbuffer(sizeInBytes);
		}

		virtual void * GetCbufferPointer(HCBUFFER cbuffer) override {
			return cbufferAllocator.GetCbufferPointer(cbuffer);
		}
		
		virtual void AddCbuffer(HITEM item, HCBUFFER cbuffer, unsigned int slot) override {
			cbufferAllocator.AddRenderItemCbuffer(renderItemColl.GetItem(item), cbuffer, slot);
		}

		virtual void SetCbuffer(HITEM item, HCBUFFER cbuffer, unsigned int idx, unsigned int slot) override {
			cbufferAllocator.SetRenderItemCbuffer(renderItemColl.GetItem(item), cbuffer, idx, slot);
		}

		virtual UINT GetCbufferSlot(HITEM item, const std::string & cbufferName) override {
			return matManager.GetCbufferSlot(renderItemColl.GetItem(item), cbufferName);
		}

		void ReleaseSwapChainResources() {
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

		void CreateSwapChainResources() {
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

			D3D12_DEPTH_STENCIL_VIEW_DESC dsvd = {};
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


	};


}
