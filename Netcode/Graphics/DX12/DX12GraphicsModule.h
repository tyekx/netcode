#pragma once

#include <Netcode/Modules.h>
#include "DX12Decl.h"
#include "DX12Includes.h"
#include "DX12CommandList.h"
#include <NetcodeFoundation/Memory.h>

namespace Netcode::Graphics::DX12 {

	class FenceImpl;
	class HeapManager;
	class ResourceContext;
	class ResourcePool;
	class ResourceViewsImpl;
	class ConstantBufferPool;
	class DynamicDescriptorHeap;
	class DebugContext;
	class CommandList;
	class CommandListPool;
	class GPipelineStateLibrary;
	class CPipelineStateLibrary;
	class InputLayoutLibrary;
	class StreamOutputLibrary;
	class RootSignatureLibrary;
	class SpriteFontLibrary;
	class ShaderLibrary;
	class TextureLibrary;


	class FrameResource {
	public:
		com_ptr<ID3D12Resource> swapChainBuffer;
		com_ptr<ID3D12GraphicsCommandList3> commandList;
		com_ptr<ID3D12CommandAllocator> commandAllocator;
		com_ptr<ID3D12Fence1> fence;
		UINT64 fenceValue;
		HANDLE fenceEvent;

		ID3D12GraphicsCommandList3 * GetCommandList() const;

		void CreateResources(ID3D12Device * device);
		void WaitForCompletion();
		void FinishRecording();
		void ReleaseResources();
	};

	class DX12GraphicsModule : public Module::IGraphicsModule, Graphics::IFrameContext {
	private:
		void CullFrameGraph(Ptr<FrameGraph> frameGraph);
		void ExecuteFrameGraph(Ref<FrameGraph> frameGraph);
	public:
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

		Ref<HeapManager> heapManager;
		Ref<ResourcePool> resourcePool;
		Ref<ConstantBufferPool> cbufferPool;
		Ref<DynamicDescriptorHeap> dheaps;

		Ref<FenceImpl> mainFence;
		Ref<FenceImpl> uploadFence;
		Ref<CommandListPool> commandListPool;

		Ref<SpriteFontLibrary> spriteFontLibrary;
		Ref<ShaderLibrary> shaderLibrary;
		Ref<RootSignatureLibrary> rootSigLibrary;
		Ref<StreamOutputLibrary> streamOutputLibrary;
		Ref<InputLayoutLibrary> inputLayoutLibrary;
		Ref<GPipelineStateLibrary> gPipelineLibrary;
		Ref<CPipelineStateLibrary> cPipelineLibrary;
		Ref<TextureLibrary> textureLibrary;

		Ref<DebugContext> debugContext;
		Ref<ResourceContext> resourceContext;
		Ref<ResourceViewsImpl> renderTargetViews;
		Ref<ResourceViewsImpl> depthStencilView;

		std::vector<CommandList> inFlightCommandLists;

		Ref<GpuResource> depthStencil;

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

		virtual void ClearImportCache() override;

		virtual void * GetSwapChain() const override;

		virtual void Prepare() override;

		virtual void Present() override;

		virtual void Start(Module::AApp * app) override;

		virtual void DeviceSync() override;

		virtual void CompleteFrame() override;

		virtual void Shutdown() override;

		virtual void OnResized(int width, int height) override;

		virtual void OnModeChanged(DisplayMode displayMode) override;

		virtual Rect GetDisplayRect() const override;

		virtual float GetAspectRatio() const override;

		virtual void SyncUpload(Ref<Netcode::Graphics::UploadBatch> upload) override;

		virtual void Run(Ref<Netcode::FrameGraph> frameGraph, FrameGraphCullMode cullMode) override;

		virtual Netcode::UInt2 GetBackbufferSize() const override;

		virtual DXGI_FORMAT GetBackbufferFormat() const override;

		virtual DXGI_FORMAT GetDepthStencilFormat() const override;

		void ReleaseSwapChainResources();

		void CreateSwapChainResources();

		virtual Ref<Fence> CreateFence(uint64_t initialValue) override;

		virtual Ref<ShaderBuilder> CreateShaderBuilder() override;

		virtual Ref<GPipelineStateBuilder> CreateGPipelineStateBuilder() override;

		virtual Ref<CPipelineStateBuilder> CreateCPipelineStateBuilder() override;

		virtual Ref<InputLayoutBuilder> CreateInputLayoutBuilder() override;

		virtual Ref<StreamOutputBuilder> CreateStreamOutputBuilder() override;

		virtual Ref<RootSignatureBuilder> CreateRootSignatureBuilder() override;

		virtual Ref<SpriteFontBuilder> CreateSpriteFontBuilder() override;

		virtual Ref<SpriteBatchBuilder> CreateSpriteBatchBuilder() override;

		virtual Ref<TextureBuilder> CreateTextureBuilder() override;

		virtual Ref<FrameGraphBuilder> CreateFrameGraphBuilder() override;

	};

}
