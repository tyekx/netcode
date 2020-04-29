#include "DX12DynamicDescriptorHeap.h"
#include "DX12ConstantBufferPool.h"
#include "DX12ResourcePool.h"
#include "DX12HeapManager.h"
#include "DX12CommandListPool.h"
#include "DX12FrameGraph.h"
#include "DX12RenderContext.h"
#include "DX12Fence.h"

namespace Netcode::Graphics::DX12 {

	/*
	Designed to work with RAII idiom, constructor should take all neccessary parameters to execute a frame
	Does not own any memory
	*/
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
		std::vector<RenderPassRef> computeStack;
		std::vector<RenderPassRef> directStack;
		ID3D12CommandList * renderTargetToPresent;
		bool usingBackbuffer;

		bool SubmitCompute() {
			if(computeSubmitCacheSize != 0) {
				computeCommandQueue->ExecuteCommandLists(computeSubmitCacheSize, computeSubmitCache);
				computeSubmitCacheSize = 0;
				return true;
			}
			return false;
		}

		bool SubmitDirect() {
			if(directSubmitCacheSize != 0) {
				directCommandQueue->ExecuteCommandLists(directSubmitCacheSize, directSubmitCache);
				directSubmitCacheSize = 0;
				return true;
			}
			return false;
		}

		void AddDirectCommandList(ID3D12CommandList * cl) {
			directSubmitCache[directSubmitCacheSize++] = cl;
		}

		void AddComputeCommandList(ID3D12CommandList * cl) {
			computeSubmitCache[computeSubmitCacheSize++] = cl;
		}

		void PushCompute(RenderPassRef renderPass, CommandList ccl) {
			computeStack.emplace_back(std::move(renderPass));
			AddComputeCommandList(ccl.commandList.Get());
			inFlightCommandLists->push_back(std::move(ccl));
		}

		void PushDirect(RenderPassRef renderPass, CommandList gcl) {
			directStack.emplace_back(std::move(renderPass));
			AddDirectCommandList(gcl.commandList.Get());
			inFlightCommandLists->push_back(std::move(gcl));
		}

		void BeginFrame() {
			if(!usingBackbuffer) {
				return;
			}

			CommandList p2rt = commandListPool->GetDirect();
			CommandList rt2p = commandListPool->GetDirect();

			GraphicsContext presentToRT{ resourcePool, cbufferPool, dheaps, p2rt.commandList,
				backbufferHandle, depthStencilHandle, viewport, scissorRect };

			const auto rt2pTransition = CD3DX12_RESOURCE_BARRIER::Transition(backbufferResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			const auto p2rtTransition = CD3DX12_RESOURCE_BARRIER::Transition(backbufferResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

			rt2p.commandList->ResourceBarrier(1, &rt2pTransition);

			p2rt.commandList->ResourceBarrier(1, &p2rtTransition);

			presentToRT.SetRenderTargets(nullptr, nullptr);
			presentToRT.SetViewport();
			presentToRT.SetScissorRect();
			presentToRT.ClearRenderTarget(0, clearColor);
			presentToRT.ClearDepthOnly();

			DX_API("Failed to close command list")
				p2rt.commandList->Close();

			DX_API("Failed to close command list")
				rt2p.commandList->Close();

			AddDirectCommandList(p2rt.commandList.Get());

			renderTargetToPresent = rt2p.commandList.Get();

			inFlightCommandLists->push_back(std::move(p2rt));
			inFlightCommandLists->push_back(std::move(rt2p));
		}
		
		void CloseFrame() {
			if(usingBackbuffer) {
				AddDirectCommandList(renderTargetToPresent);
			}

			ASSERT((computeSubmitCacheSize == 0 && directSubmitCacheSize != 0) || 
				   (computeSubmitCacheSize != 0 && directSubmitCacheSize == 0), "A frame can not end with both direct and compute work remaining");

			if(SubmitDirect()) {
				mainFence->Signal(directCommandQueue);
			}

			if(SubmitCompute()) {
				mainFence->Signal(computeCommandQueue);
			}
		}

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
			D3D12_RECT scissorRect) :
				commandListPool{ commandListPool },
				heapManager{ heapManager },
				resourcePool{ resourcePool },
				dheaps{ dheaps },
				cbufferPool{ cbufferPool },
				directCommandQueue{ directCommandQueue },
				computeCommandQueue{ computeCommandQueue },
				backbufferResource{ backbufferResource },
				inFlightCommandLists{ inFlightCommandLists },
				clearColor{ clearColor },
				mainFence{ mainFence },
				backbufferHandle{ backbufferHandle },
				depthStencilHandle{ depthStencilHandle },
				viewport{ viewport },
				scissorRect{ scissorRect },
				directSubmitCache{},
				computeSubmitCache{},
				directSubmitCacheSize{ 0 },
				computeSubmitCacheSize{ 0 },
				computeStack{},
				directStack{},
				renderTargetToPresent{ nullptr },
				usingBackbuffer{ false } {
		}

		bool HasMatchingElement(ArrayView<uint64_t> lhs, ArrayView<uint64_t> rhs) {
			for(size_t i = 0; i < lhs.Size(); ++i) {
				for(size_t j = 0; j < rhs.Size(); ++j) {
					if(lhs[j] == rhs[i]) {
						return true;
					}
				}
			}
			return false;
		}

		bool HasResourceReadDependency(RenderPassRef renderPass) {
			if(renderPass->Type() == RenderPassType::DIRECT) {
				ArrayView<uint64_t> readResources = renderPass->GetReadResources();
				for(const auto & computePass : computeStack) {
					ArrayView<uint64_t> writtenResources = computePass->GetWrittenResources();

					if(HasMatchingElement(readResources, writtenResources)) {
						return true;
					}
				}
			}

			if(renderPass->Type() == RenderPassType::COMPUTE) {
				ArrayView<uint64_t> readResources = renderPass->GetReadResources();
				for(const auto & directPass : directStack) {
					ArrayView<uint64_t> writtenResources = directPass->GetWrittenResources();

					if(HasMatchingElement(readResources, writtenResources)) {
						return true;
					}
				}
			}

			return false;
		}

		void InvokeRenderFunction(RenderPassRef renderPass) {
			if(renderPass->Type() == RenderPassType::DIRECT) {
				CommandList cl = commandListPool->GetDirect();
				GraphicsContext gctx{ resourcePool, cbufferPool, dheaps, cl.commandList,
					backbufferHandle, depthStencilHandle, viewport, scissorRect };

				gctx.BeginPass();
				renderPass->Render(&gctx);
				gctx.EndPass();

				DX_API("Failed to close command list during render pass: %s", renderPass->name.c_str())
					cl.commandList->Close();

				PushDirect(renderPass, std::move(cl));
			}

			if(renderPass->Type() == RenderPassType::COMPUTE) {
				CommandList cl = commandListPool->GetCompute();
				ComputeContext cctx{ resourcePool, cbufferPool, dheaps, cl.commandList };

				cctx.BeginPass();
				renderPass->Render(&cctx);
				cctx.EndPass();

				DX_API("Failed to close command list during render pass: %s", renderPass->name.c_str())
					cl.commandList->Close();

				PushCompute(renderPass, std::move(cl));
			}
		}

		void Execute(FrameGraphRef frameGraph) {
			usingBackbuffer = frameGraph->UsingBackbuffer();

			BeginFrame();

			std::vector<RenderPassRef> runnable = frameGraph->QueryCompleteRenderPasses();

			while(!runnable.empty()) {
				bool directSyncSubmission = false;
				bool computeSyncSubmission = false;

				for(auto & rp : runnable) {
					// step#1: determine dependencies
					if(HasResourceReadDependency(rp)) {
						if(rp->Type() == RenderPassType::DIRECT) {
							computeSyncSubmission = true;
						}

						if(rp->Type() == RenderPassType::COMPUTE) {
							directSyncSubmission = true;
						}
					}

					// step#2:
					InvokeRenderFunction(rp);
				}
				
				// step#3: process submissions
				if(directSyncSubmission) {
					SubmitDirect();
					mainFence->Signal(directCommandQueue);
					mainFence->Wait(computeCommandQueue);
				}

				if(computeSyncSubmission) {
					SubmitCompute();
					mainFence->Signal(computeCommandQueue);
					mainFence->Wait(directCommandQueue);
				}

				frameGraph->EraseRenderPasses(std::move(runnable));
				runnable = frameGraph->QueryCompleteRenderPasses();
			}
		}

		~FrameGraphExecutor() {
			CloseFrame();
		}


	};

}
