#pragma once

#include "Utility.h"
#include "Modules.h"
#include "GraphicsContexts.h"
#include <map>

#include "DX12Common.h"
#include "DX12Fence.h"
#include "DX12SpriteFontLibrary.h"
#include "DX12ConstantBufferPool.h"
#include "DX12ResourcePool.h"
#include "DX12ResourceContext.h"
#include "DX12RenderContext.h"
#include "DX12DynamicDescriptorHeap.h"

#include "DX12ShaderLibrary.h"
#include "DX12InputLayoutLibrary.h"
#include "DX12RootSignatureLibrary.h"
#include "DX12StreamOutputLibrary.h"
#include "DX12GPipelineStateLibrary.h"
#include "DX12CPipelineStateLibrary.h"

#include "GraphicsContexts.h"


namespace Egg::Graphics::DX12 {

	class FrameResource {
	public:
		com_ptr<ID3D12Resource> swapChainBuffer;
		com_ptr<ID3D12GraphicsCommandList2> commandList;
		com_ptr<ID3D12CommandAllocator> commandAllocator;
		com_ptr<ID3D12Fence1> fence;
		UINT64 fenceValue;
		HANDLE fenceEvent;

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
			fenceValue = 1;

			DX_API("Failed to create fence")
				device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));
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

	class DX12GraphicsModule : public Egg::Module::IGraphicsModule, Egg::Graphics::IFrameContext {
	protected:
		Egg::Module::AppEventSystem * eventSystem;
		HWND hwnd;
		com_ptr<ID3D12Debug3> debugController;

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

		DX12ResourceViewsRef renderTargetViews;
		DX12ResourceViewsRef depthStencilView;

		uint64_t depthStencil;

		UINT backbufferDepth;
		DirectX::XMUINT2 lastWindowedModeSize;
		UINT width;
		UINT height;

		D3D12_VIEWPORT viewport;
		D3D12_RECT scissorRect;

		DXGI_FORMAT renderTargetFormat;
		DXGI_FORMAT depthStencilFormat;

		std::vector<FrameResource> frameResources;
		UINT backbufferIndex;
		UINT presentedBackbufferIndex;

		float aspectRatio;
		DXGI_RGBA clearColor;

		DisplayMode displayMode;

		void NextBackBufferIndex();

		void CreateFactory();

		void CreateDevice();

		void ClearAdapters();

		void QueryAdapters();

		void CreateCommandQueue();

		void QuerySyncSupport();

		void CreateLibraries();

		virtual void CreateSwapChain();

		void SetContextReferences();

		void CreateContexts();

		void UpdateViewport();

		DX12SpriteFontLibraryRef spriteFontLibrary;
		DX12ShaderLibraryRef shaderLibrary;
		DX12RootSignatureLibraryRef rootSigLibrary;
		DX12StreamOutputLibraryRef streamOutputLibrary;
		DX12InputLayoutLibraryRef inputLayoutLibrary;
		DX12GPipelineStateLibraryRef gPipelineLibrary;
		DX12CPipelineStateLibraryRef cPipelineLibrary;

		HeapManager heapManager;
		ResourcePool resourcePool;
		ConstantBufferPool cbufferPool;
		DynamicDescriptorHeap dheaps;

		RenderContext renderContext;
		ResourceContext resourceContext;

	public:

		virtual void * GetSwapChain() const override;

		virtual void Prepare() override;

		virtual void Render() override;

		virtual void Present() override;

		virtual void Start(Module::AApp * app) override;

		virtual void Shutdown() override;

		virtual void OnResized(int width, int height) override;

		virtual void OnModeChanged(DisplayMode displayMode) override;

		virtual RECT GetDisplayRect() const override;

		virtual float GetAspectRatio() const override;

		virtual void SyncUpload(const UploadBatch & upload) override;

		virtual DirectX::XMUINT2 GetBackbufferSize() const override;

		void ReleaseSwapChainResources();

		void CreateSwapChainResources();

		virtual FenceRef CreateFence(uint64_t initialValue) const override;

		virtual ShaderBuilderRef CreateShaderBuilder() const override;

		virtual GPipelineStateBuilderRef CreateGPipelineStateBuilder() const override;

		virtual CPipelineStateBuilderRef CreateCPipelineStateBuilder() const override;

		virtual InputLayoutBuilderRef CreateInputLayoutBuilder() const override;

		virtual StreamOutputBuilderRef CreateStreamOutputBuilder() const override;

		virtual RootSignatureBuilderRef CreateRootSignatureBuilder() const override;

		virtual SpriteFontBuilderRef CreateSpriteFontBuilder() const override;

		virtual SpriteBatchBuilderRef CreateSpriteBatchBuilder() const override;

		virtual TextureBuilderRef CreateTextureBuilder() const override;

	};


}
