#pragma once

#include "Utility.h"
#include "Modules.h"
#include "GraphicsContexts.h"
#include <map>

#include "DX12Common.h"
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

#include "GraphicsContexts.h"


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

		std::unique_ptr<SpriteBatch> spriteBatch;
		float aspectRatio;

		void NextBackBufferIndex();

		void CreateFactory();

		void CreateDevice();

		void ClearAdapters();

		void QueryAdapters();

		void CreateCommandQueue();

		void QuerySyncSupport();

		void CreateSwapChain();

		void CreateLibraries();

		void SetContextReferences();

		void CreateContexts();

		DX12ShaderLibraryRef shaderLibrary;
		DX12RootSignatureLibraryRef rootSigLibrary;
		DX12StreamOutputLibraryRef streamOutputLibrary;
		DX12InputLayoutLibraryRef inputLayoutLibrary;
		DX12GPipelineStateLibraryRef gPipelineLibrary;

		HeapManager heapManager;
		ResourcePool resourcePool;
		ConstantBufferPool cbufferPool;
		DynamicDescriptorHeap dheaps;

		RenderContext renderContext;
		ResourceContext resourceContext;

	public:

		virtual void Prepare() override;

		virtual void Render() override;

		virtual void Present() override;

		virtual void Start(Module::AApp * app) override;

		virtual void Shutdown() override;

		virtual void OnResized(int width, int height) override;

		virtual float GetAspectRatio() const override;

		virtual void SyncUpload(const UploadBatch & upload) override;

		virtual DirectX::XMUINT2 GetBackbufferSize() const override;

		void ReleaseSwapChainResources();

		void CreateSwapChainResources();

		virtual ShaderBuilderRef CreateShaderBuilder() const override;

		virtual GPipelineStateBuilderRef CreateGPipelineStateBuilder() const override;

		virtual InputLayoutBuilderRef CreateInputLayoutBuilder() const override;

		virtual StreamOutputBuilderRef CreateStreamOutputBuilder() const override;

		virtual RootSignatureBuilderRef CreateRootSignatureBuilder() const override;
	};


}
