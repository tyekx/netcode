#pragma once

#include <NetcodeFoundation/Memory.h>

#include "../../Utility.h"
#include "../../Modules.h"
#include "../../BulkAllocator.hpp"
#include "../GraphicsContexts.h"

#include <map>
#include <variant>

#include "DX12Common.h"
#include "DX12ConstantBufferPool.h"
#include "DX12ResourcePool.h"
#include "DX12DynamicDescriptorHeap.h"
#include "DX12CommandList.h"

namespace Netcode::Graphics::DX12 {

	class Fence;
	class HeapManager;
	class ResourceContext;
	class DebugContext;
	class CommandListPool;
	class GPipelineStateLibrary;
	class CPipelineStateLibrary;
	class InputLayoutLibrary;
	class StreamOutputLibrary;
	class RootSignatureLibrary;
	class SpriteFontLibrary;
	class ShaderLibrary;

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
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			commandList->ResourceBarrier(1, &barrier);

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

	class DX12GraphicsModule : public Module::IGraphicsModule, Graphics::IFrameContext {
	private:
		void CullFrameGraph(FrameGraphRef frameGraph);
		void ExecuteFrameGraph(FrameGraphRef frameGraph);

	protected:
		Module::AppEventSystem * eventSystem;
		HWND hwnd;
		com_ptr<ID3D12Debug3> debugController;

		com_ptr<IDXGIFactory5> dxgiFactory;

		Memory::StackAllocator stackAllocator;
		Memory::ObjectAllocator objectAllocator;

		com_ptr<IDXGISwapChain4> swapChain;
		UINT swapChainFlags;

		com_ptr<ID3D12Device5> device;
		com_ptr<ID3D12CommandQueue> commandQueue;
		com_ptr<ID3D12CommandQueue> computeCommandQueue;
		com_ptr<ID3D12CommandQueue> copyCommandQueue;
		
		com_ptr<IDXGIAdapter3> adapters[8];
		UINT adaptersLength;

		com_ptr<IDXGIOutput1> outputs[8];
		UINT outputsLength;

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

		ResourcePool resourcePool;
		ConstantBufferPool cbufferPool;
		DynamicDescriptorHeap dheaps;


		std::shared_ptr<DX12::Fence> mainFence;
		std::shared_ptr<DX12::Fence> uploadFence;
		std::shared_ptr<DX12::CommandListPool> commandListPool;
		std::shared_ptr<DX12::HeapManager> heapManager;

		std::shared_ptr<DX12::SpriteFontLibrary> spriteFontLibrary;
		std::shared_ptr<DX12::ShaderLibrary> shaderLibrary;
		std::shared_ptr<DX12::RootSignatureLibrary> rootSigLibrary;
		std::shared_ptr<DX12::StreamOutputLibrary> streamOutputLibrary;
		std::shared_ptr<DX12::InputLayoutLibrary> inputLayoutLibrary;
		std::shared_ptr<DX12::GPipelineStateLibrary> gPipelineLibrary;
		std::shared_ptr<DX12::CPipelineStateLibrary> cPipelineLibrary;

		std::shared_ptr<DX12::DebugContext> debugContext;
		std::shared_ptr<DX12::ResourceContext> resourceContext;
		std::shared_ptr<DX12::ResourceViews> renderTargetViews;
		std::shared_ptr<DX12::ResourceViews> depthStencilView;

		std::vector<CommandList> inFlightCommandLists;

		GpuResourceRef depthStencil;

		void NextBackBufferIndex();

		void CreateFactory();

		void CreateDevice();

		void ClearAdapters();

		void QueryAdapters();

		void CreateCommandQueue();

		void CreateFences();

		void QuerySyncSupport();

		void CreateLibraries();

		virtual void CreateSwapChain();

		void SetContextReferences();

		void CreateContexts();

		void UpdateViewport();

		void CreateDebugContext();

	public:

		virtual void * GetSwapChain() const override;

		virtual void Prepare() override;

		virtual void Present() override;

		virtual void Start(Module::AApp * app) override;

		virtual void DeviceSync() override;

		virtual void CompleteFrame() override;

		virtual void Shutdown() override;

		virtual void OnResized(int width, int height) override;

		virtual void OnModeChanged(DisplayMode displayMode) override;

		virtual RECT GetDisplayRect() const override;

		virtual float GetAspectRatio() const override;

		virtual void SyncUpload(const UploadBatch & upload) override;

		virtual void Run(FrameGraphRef frameGraph, FrameGraphCullMode cullMode) override;

		virtual Netcode::UInt2 GetBackbufferSize() const override;

		virtual DXGI_FORMAT GetBackbufferFormat() const override;

		virtual DXGI_FORMAT GetDepthStencilFormat() const override;

		void ReleaseSwapChainResources();

		void CreateSwapChainResources();

		virtual FenceRef CreateFence(uint64_t initialValue) override;

		virtual ShaderBuilderRef CreateShaderBuilder() override;

		virtual GPipelineStateBuilderRef CreateGPipelineStateBuilder() override;

		virtual CPipelineStateBuilderRef CreateCPipelineStateBuilder() override;

		virtual InputLayoutBuilderRef CreateInputLayoutBuilder() override;

		virtual StreamOutputBuilderRef CreateStreamOutputBuilder() override;

		virtual RootSignatureBuilderRef CreateRootSignatureBuilder() override;

		virtual SpriteFontBuilderRef CreateSpriteFontBuilder() override;

		virtual SpriteBatchBuilderRef CreateSpriteBatchBuilder() override;

		virtual TextureBuilderRef CreateTextureBuilder() override;

		virtual FrameGraphBuilderRef CreateFrameGraphBuilder() override;

	};

}
