#pragma once

#include "DX12Common.h"
#include "GraphicsContexts.h"
#include "DX12ResourcePool.h"
#include "DX12ConstantBufferPool.h"
#include "DX12DynamicDescriptorHeap.h"

namespace Egg::Graphics::DX12 {

	class RenderContext : public Egg::Graphics::IRenderContext {
		D3D12_CPU_DESCRIPTOR_HANDLE currentlyBoundDepth;
		D3D12_CPU_DESCRIPTOR_HANDLE currentlyBoundRenderTargets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE backbuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE backbufferDepth;
		D3D12_VIEWPORT defaultViewport;
		ID3D12GraphicsCommandList * gcl;
		ResourcePool * resources;
		ConstantBufferPool * cbuffers;
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		D3D12_GPU_VIRTUAL_ADDRESS streamOutput_FilledSizeLocation;
		DynamicDescriptorHeap * descHeaps;

		virtual void SetStencilReference(uint8_t stencilValue) override;

		virtual void SetVertexBuffer(uint64_t handle) override;

		virtual void SetIndexBuffer(uint64_t handle) override;

		virtual void DrawIndexed(uint64_t indexCount) override;

		virtual void Draw(uint64_t vertexCount) override;

		virtual void SetRootSignature(RootSignatureRef rs) override;

		virtual void SetPipelineState(PipelineStateRef pso) override;

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) override;

		virtual void ClearUnorderedAccessViewUint(uint64_t handle, const DirectX::XMUINT4 & values) override;


		virtual void ClearDepthOnly() override;
		virtual void ClearStencilOnly() override;
		virtual void ClearDepthStencil() override;

		virtual void ClearRenderTarget(uint8_t idx) override;

		virtual void SetStreamOutput(uint64_t handle) override;

		virtual void SetStreamOutputFilledSize(uint64_t handle, uint64_t byteOffset) override;

		virtual void ResetStreamOutput() override;

		virtual void SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) override;

		virtual void SetViewport(uint32_t width, uint32_t height) override;

		virtual void SetViewport() override;

		virtual void SetRenderTargets(ResourceViewsRef renderTargets, ResourceViewsRef depthStencil) override;

		virtual void SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil) override;

		virtual void SetRenderTargets(uint64_t renderTarget, uint64_t depthStencil) override;

		virtual void SetShaderResources(int slot, std::initializer_list<uint64_t> shaderResourceHandles) override;

		virtual void SetConstantBuffer(int slot, uint64_t cbufferHandle) override;

		virtual void SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes) override;

		virtual void ResourceBarrier(uint64_t handle, ResourceState before, ResourceState after) override;

		virtual void FlushResourceBarriers() override;

		virtual void BeginPass() override;

		virtual void EndPass() override;

	};

}
