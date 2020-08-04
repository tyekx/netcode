#include "DX12DynamicDescriptorHeap.h"
#include "DX12ConstantBufferPool.h"
#include "DX12ResourcePool.h"
#include "DX12HeapManager.h"
#include "DX12CommandListPool.h"
#include "DX12FrameGraph.h"
#include "DX12RenderContext.h"
#include "DX12Fence.h"

namespace Netcode::Graphics::DX12 {

	class FrameGraphExecutor {
		CommandListPool * commandListPool;
		HeapManager * heapManager;
		ResourcePool * resourcePool;
		DynamicDescriptorHeap * dheaps;
		ConstantBufferPool * cbufferPool;
		ID3D12CommandQueue * directCommandQueue;
		ID3D12CommandQueue * computeCommandQueue;
		ID3D12Resource * backbufferResource;
		std::vector<CommandList> * inFlightCommandLists;
		const float * clearColor;
		DX12FenceRef mainFence;
		D3D12_CPU_DESCRIPTOR_HANDLE backbufferHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle;
		D3D12_VIEWPORT viewport;
		D3D12_RECT scissorRect;
		ID3D12CommandList * directSubmitCache[16];
		ID3D12CommandList * computeSubmitCache[16];
		uint32_t directSubmitCacheSize;
		uint32_t computeSubmitCacheSize;
		std::vector<Ref<RenderPass>> computeStack;
		std::vector<Ref<RenderPass>> directStack;
		ID3D12CommandList * renderTargetToPresent;
		bool usingBackbuffer;

		bool SubmitCompute();

		bool SubmitDirect();

		void AddDirectCommandList(ID3D12CommandList * cl);

		void AddComputeCommandList(ID3D12CommandList * cl);

		void PushCompute(Ref<Netcode::RenderPass> renderPass, CommandList ccl);

		void PushDirect(Ref<Netcode::RenderPass> renderPass, CommandList gcl);

		void BeginFrame();
		
		void CloseFrame();

		bool HasMatchingElement(ArrayView<uint64_t> lhs, ArrayView<uint64_t> rhs);

		bool HasResourceReadDependency(Ref<Netcode::RenderPass> renderPass);

		void InvokeRenderFunction(Ref<Netcode::RenderPass> renderPass);

	public:

		explicit FrameGraphExecutor(
			CommandListPool * commandListPool,
			HeapManager * heapManager,
			ResourcePool * resourcePool,
			DynamicDescriptorHeap * dheaps,
			ConstantBufferPool * cbufferPool,
			ID3D12CommandQueue * directCommandQueue,
			ID3D12CommandQueue * computeCommandQueue,
			ID3D12Resource * backbufferResource,
			std::vector<CommandList> * inFlightCommandLists,
			const float * clearColor,
			DX12FenceRef mainFence,
			D3D12_CPU_DESCRIPTOR_HANDLE backbufferHandle,
			D3D12_CPU_DESCRIPTOR_HANDLE depthStencilHandle,
			D3D12_VIEWPORT viewport,
			D3D12_RECT scissorRect);

		void Execute(Ref<FrameGraph> frameGraph);

		~FrameGraphExecutor();

	};

}
