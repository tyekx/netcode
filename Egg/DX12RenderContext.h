#pragma once

#include "DX12Common.h"
#include "GraphicsContexts.h"
#include "DX12ResourcePool.h"
#include "DX12ConstantBufferPool.h"
#include "DX12DynamicDescriptorHeap.h"

namespace Egg::Graphics::DX12 {

	class BaseRenderContext : public Egg::Graphics::IRenderContext {
	protected:
		ResourcePool * resources;
		ConstantBufferPool * cbuffers;
		DynamicDescriptorHeap * descHeaps;
		com_ptr<ID3D12GraphicsCommandList> commandList;
		FenceRef signalFence;
		std::vector<FenceRef> waitFences;
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
	public:

		BaseRenderContext(ResourcePool * resourcePool, ConstantBufferPool * cbufferPool, DynamicDescriptorHeap * dHeaps, com_ptr<ID3D12GraphicsCommandList> cl);

		virtual ~BaseRenderContext() = default;

		virtual void ResourceBarrier(uint64_t handle, ResourceState before, ResourceState after) override;

		virtual void FlushResourceBarriers() override;

		FenceRef GetSignalFence() const {
			return signalFence;
		}

		std::vector<FenceRef> GetWaitFences() const {
			return waitFences;
		}

		com_ptr<ID3D12GraphicsCommandList> GetCommandListRef() const {
			return commandList;
		}

	};


	class ComputeContext : public BaseRenderContext {
	public:
		using BaseRenderContext::BaseRenderContext;



		// Inherited via BaseRenderContext
		virtual void SetRootSignature(RootSignatureRef rs) override;

		virtual void SetPipelineState(PipelineStateRef pso) override;

		virtual void SetVertexBuffer(uint64_t handle) override;

		virtual void SetIndexBuffer(uint64_t handle) override;

		virtual void DrawIndexed(uint32_t indexCount) override;

		virtual void DrawIndexed(uint32_t indexCount, uint32_t vertexOffset) override;

		virtual void Draw(uint32_t vertexCount) override;

		virtual void Draw(uint32_t vertexCount, uint32_t vertexOffset) override;

		virtual void Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ) override;

		virtual void Signal(FenceRef fence) override;

		virtual void Wait(FenceRef fence) override;

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) override;

		virtual void ClearUnorderedAccessViewUint(uint64_t handle, const DirectX::XMUINT4 & values) override;

		virtual void ClearRenderTarget(uint8_t idx) override;

		virtual void ClearRenderTarget(uint8_t idx, const float * clearColor) override;

		virtual void ClearDepthOnly() override;

		virtual void ClearStencilOnly() override;

		virtual void ClearDepthStencil() override;

		virtual void SetStreamOutput(uint64_t handle) override;

		virtual void SetStreamOutputFilledSize(uint64_t handle, uint64_t byteOffset) override;

		virtual void ResetStreamOutput() override;

		virtual void SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil) override;

		virtual void SetStencilReference(uint8_t stencilValue) override;

		virtual void SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) override;

		virtual void SetViewport(uint32_t width, uint32_t height) override;

		virtual void SetViewport() override;

		virtual void SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) override;

		virtual void SetScissorRect(uint32_t width, uint32_t height) override;

		virtual void SetScissorRect() override;

		virtual void SetRenderTargets(uint64_t renderTarget, uint64_t depthStencil) override;

		virtual void SetRenderTargets(ResourceViewsRef renderTargets, ResourceViewsRef depthStencil) override;

		virtual void SetConstantBuffer(int slot, uint64_t cbufferHandle) override;

		virtual void SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes) override;

		virtual void SetShaderResources(int slot, std::initializer_list<uint64_t> shaderResourceHandles) override;

		virtual void SetShaderResources(int slot, ResourceViewsRef resourceView) override;

		virtual void BeginPass() override;

		virtual void EndPass() override;

	};

	class GraphicsContext : public BaseRenderContext {
		D3D12_CPU_DESCRIPTOR_HANDLE currentlyBoundDepth;
		D3D12_CPU_DESCRIPTOR_HANDLE currentlyBoundRenderTargets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		D3D12_GPU_VIRTUAL_ADDRESS streamOutput_FilledSizeLocation;
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE backbuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE backbufferDepth;
		D3D12_VIEWPORT defaultViewport;
		D3D12_RECT defaultScissorRect;

		GraphicsContext(
			ResourcePool * resourcePool,
			ConstantBufferPool * cbpool,
			DynamicDescriptorHeap * dheaps,
			com_ptr<ID3D12GraphicsCommandList> directCommandList,
			const D3D12_CPU_DESCRIPTOR_HANDLE & backbuffer,
			const D3D12_CPU_DESCRIPTOR_HANDLE & backbufferDepth,
			const D3D12_VIEWPORT & viewPort,
			const D3D12_RECT & scissorRect
		);

		virtual void SetStencilReference(uint8_t stencilValue) override;

		virtual void SetVertexBuffer(uint64_t handle) override;

		virtual void SetIndexBuffer(uint64_t handle) override;

		virtual void DrawIndexed(uint32_t indexCount) override;

		virtual void DrawIndexed(uint32_t indexCount, uint32_t vertexOffset) override;

		virtual void Draw(uint32_t vertexCount) override;

		virtual void Draw(uint32_t vertexCount, uint32_t vertexOffset) override;

		virtual void Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ) override;

		virtual void Signal(FenceRef fence) override;

		virtual void Wait(FenceRef fence) override;

		virtual void SetRootSignature(RootSignatureRef rs) override;

		virtual void SetPipelineState(PipelineStateRef pso) override;

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) override;

		virtual void ClearUnorderedAccessViewUint(uint64_t handle, const DirectX::XMUINT4 & values) override;

		virtual void ClearDepthOnly() override;
		virtual void ClearStencilOnly() override;
		virtual void ClearDepthStencil() override;

		virtual void ClearRenderTarget(uint8_t idx) override;
		virtual void ClearRenderTarget(uint8_t idx, const float * clearColor) override;

		virtual void SetStreamOutput(uint64_t handle) override;

		virtual void SetStreamOutputFilledSize(uint64_t handle, uint64_t byteOffset) override;
		virtual void ResetStreamOutput() override;

		virtual void SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) override;
		virtual void SetViewport(uint32_t width, uint32_t height) override;
		virtual void SetViewport() override;

		virtual void SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) override;
		virtual void SetScissorRect(uint32_t width, uint32_t height) override;
		virtual void SetScissorRect() override;

		virtual void SetRenderTargets(ResourceViewsRef renderTargets, ResourceViewsRef depthStencil) override;
		virtual void SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil) override;
		virtual void SetRenderTargets(uint64_t renderTarget, uint64_t depthStencil) override;

		virtual void SetShaderResources(int slot, std::initializer_list<uint64_t> shaderResourceHandles) override;
		virtual void SetShaderResources(int slot, ResourceViewsRef resourceView) override;

		virtual void SetConstantBuffer(int slot, uint64_t cbufferHandle) override;
		virtual void SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes) override;

		virtual void BeginPass() override;

		virtual void EndPass() override;

	};

}
