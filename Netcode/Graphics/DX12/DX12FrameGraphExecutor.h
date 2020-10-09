#pragma once

#include <Netcode/HandleDecl.h>
#include <Netcode/Graphics/FrameGraph.h>
#include "DX12CommandList.h"
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	class CommandListPool;
	class HeapManager;
	class ResourcePool;
	class DynamicDescriptorHeap;
	class ConstantBufferPool;
	class FenceImpl;

	class FrameGraphExecutor {
		Ptr<CommandListPool> commandListPool;
		Ptr<HeapManager> heapManager;
		Ptr<ResourcePool> resourcePool;
		Ptr<DynamicDescriptorHeap> dheaps;
		Ptr<ConstantBufferPool> cbufferPool;
		Ptr<IDebugContext> debugContext;
		Ptr<ID3D12CommandQueue> directCommandQueue;
		Ptr<ID3D12CommandQueue> computeCommandQueue;
		Ptr<ID3D12Resource> backbufferResource;
		std::vector<CommandList> * inFlightCommandLists;
		const float * clearColor;
		Ref<FenceImpl> mainFence;
		D3D12_CPU_DESCRIPTOR_HANDLE backbufferHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle;
		D3D12_VIEWPORT viewport;
		D3D12_RECT scissorRect;
		Ptr<ID3D12CommandList> directSubmitCache[16];
		Ptr<ID3D12CommandList> computeSubmitCache[16];
		uint32_t directSubmitCacheSize;
		uint32_t computeSubmitCacheSize;
		std::vector<Ref<RenderPass>> computeStack;
		std::vector<Ref<RenderPass>> directStack;
		Ptr<ID3D12CommandList> renderTargetToPresent;
		bool usingBackbuffer;

		bool SubmitCompute();

		bool SubmitDirect();

		void AddDirectCommandList(ID3D12CommandList * cl);

		void AddComputeCommandList(ID3D12CommandList * cl);

		void PushCompute(Ref<RenderPass> renderPass, CommandList ccl);

		void PushDirect(Ref<RenderPass> renderPass, CommandList gcl);

		void BeginFrame();

		void CloseFrame();

		bool HasMatchingElement(ArrayView<uint64_t> lhs, ArrayView<uint64_t> rhs);

		bool HasResourceReadDependency(Ref<RenderPass> renderPass);

		void InvokeRenderFunction(Ref<RenderPass> renderPass);

	public:

		explicit FrameGraphExecutor(
			Ptr<CommandListPool> commandListPool,
			Ptr<HeapManager> heapManager,
			Ptr<ResourcePool> resourcePool,
			Ptr<DynamicDescriptorHeap> dheaps,
			Ptr<ConstantBufferPool> cbufferPool,
			Ptr<IDebugContext> debugContext,
			Ptr<ID3D12CommandQueue> directCommandQueue,
			Ptr<ID3D12CommandQueue> computeCommandQueue,
			Ptr<ID3D12Resource> backbufferResource,
			Ptr<std::vector<CommandList>> inFlightCommandLists,
			const float * clearColor,
			Ref<Fence> mainFence,
			D3D12_CPU_DESCRIPTOR_HANDLE backbufferHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle,
			D3D12_VIEWPORT viewport,
			D3D12_RECT scissorRect);

		void Execute(Ref<FrameGraph> frameGraph);

		~FrameGraphExecutor();

	};

}
