#pragma once

#include <NetcodeFoundation/Math.h>
#include <Netcode/Graphics/GraphicsContexts.h>
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	class ConstantBufferPool;
	class ResourcePool;
	class DynamicDescriptorHeap;

	class BaseRenderContext : public Netcode::Graphics::IRenderContext {
	public:
		Ptr<ResourcePool> resources;
		Ptr<ConstantBufferPool> cbuffers;
		Ptr<DynamicDescriptorHeap> descHeaps;
		Ptr<ID3D12GraphicsCommandList> commandList;
		std::vector<D3D12_RESOURCE_BARRIER> barriers;

		BaseRenderContext(Ptr<ResourcePool> resourcePool,
			Ptr<ConstantBufferPool> cbufferPool,
			Ptr<DynamicDescriptorHeap> dHeaps,
			Ptr<ID3D12GraphicsCommandList> cl);

		virtual ~BaseRenderContext() = default;

		virtual void UnorderedAccessBarrier(Ref<GpuResource> handle) override;
		virtual void ResourceBarrier(Ref<GpuResource> handle, ResourceStates before, ResourceStates after) override;

		virtual void FlushResourceBarriers() override;
	};


	class ComputeContext : public BaseRenderContext {
	public:
		using BaseRenderContext::BaseRenderContext;

		// Inherited via BaseRenderContext
		virtual void SetRootSignature(Ref<Netcode::RootSignature> rs) override;

		virtual void SetPipelineState(Ref<Netcode::PipelineState> pso) override;

		virtual void SetVertexBuffer(Ref<GpuResource> handle) override;

		virtual void SetIndexBuffer(Ref<GpuResource> handle) override;

		virtual void DrawIndexed(uint32_t indexCount) override;

		virtual void DrawIndexed(uint32_t indexCount, uint32_t vertexOffset) override;

		virtual void Draw(uint32_t vertexCount) override;

		virtual void Draw(uint32_t vertexCount, uint32_t vertexOffset) override;

		virtual void Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ) override;

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) override;

		virtual void ClearUnorderedAccessViewUint(Ref<GpuResource> handle, const UInt4 & values) override;

		virtual void ClearRenderTarget(uint8_t idx) override;

		virtual void ClearRenderTarget(uint8_t idx, const float * clearColor) override;

		virtual void ClearDepthOnly() override;

		virtual void ClearStencilOnly() override;

		virtual void ClearDepthStencil() override;

		virtual void SetStreamOutput(Ref<GpuResource> handle) override;

		virtual void SetStreamOutputFilledSize(Ref<GpuResource> handle, uint64_t byteOffset) override;

		virtual void ResetStreamOutput() override;

		virtual void SetRenderTargets(std::initializer_list<Ref<GpuResource>> handles, Ref<GpuResource> depthStencil) override;

		virtual void SetStencilReference(uint8_t stencilValue) override;

		virtual void SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) override;

		virtual void SetViewport(uint32_t width, uint32_t height) override;

		virtual void SetViewport() override;

		virtual Float4 GetViewport() override;

		virtual void SetScissorRect(int32_t left, int32_t right, int32_t top, int32_t bottom) override;

		virtual void SetScissorRect(uint32_t width, uint32_t height) override;

		virtual void SetScissorRect() override;

		virtual void SetRenderTargets(std::nullptr_t rt, std::nullptr_t ds) override;

		virtual void SetRenderTargets(std::nullptr_t rt, Ref<Netcode::ResourceViews> ds) override;
		virtual void SetRenderTargets(Ref<Netcode::ResourceViews> rt, std::nullptr_t ds) override;
		virtual void SetRenderTargets(Ref<Netcode::ResourceViews> renderTargets, Ref<Netcode::ResourceViews> depthStencil) override;

		virtual void SetRenderTargets(std::nullptr_t rt, Ref<GpuResource> ds) override;
		virtual void SetRenderTargets(Ref<GpuResource> rt, std::nullptr_t ds) override;
		virtual void SetRenderTargets(Ref<GpuResource> renderTarget, Ref<GpuResource> depthStencil) override;

		virtual void SetRootConstants(int slot, const void * srcData, uint32_t numConstants) override;
		virtual void SetRootConstants(int slot, const void * srcData, uint32_t num32bitConstants, uint32_t offsetIn32BitConstants) override;
		virtual void SetConstantBuffer(int slot, Ref<GpuResource> cbufferHandle) override;
		virtual void SetConstants(int slot, uint64_t constantHandle) override;
		virtual uint64_t SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes) override;

		virtual void SetShaderResources(int slot, std::initializer_list<Ref<GpuResource>> shaderResourceHandles) override;
		virtual void SetShaderResources(int slot, Ref<Netcode::ResourceViews> resourceView) override;
		virtual void SetShaderResources(int slot, Ref<Netcode::ResourceViews> resourceView, int descriptorOffset) override;
		virtual void SetShaderResources(int slot, Ref<GpuResource> nonTextureResource) override;

		virtual void CopyBufferRegion(Ref<GpuResource> dstResource, Ref<GpuResource> srcResource, size_t sizeInBytes) override;
		
		virtual void CopyBufferRegion(Ref<GpuResource> dstResource, size_t dstOffset, Ref<GpuResource> srcResource, size_t srcOffset, size_t sizeInBytes) override;

		virtual void BeginPass() override;

		virtual void EndPass() override;

	};

	class GraphicsContext : public BaseRenderContext {
		D3D12_CPU_DESCRIPTOR_HANDLE currentlyBoundDepth;
		D3D12_CPU_DESCRIPTOR_HANDLE currentlyBoundRenderTargets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		D3D12_GPU_VIRTUAL_ADDRESS streamOutput_FilledSizeLocation;
		D3D12_VIEWPORT currentlyBoundViewport;
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE backbuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE backbufferDepth;
		D3D12_VIEWPORT defaultViewport;
		D3D12_RECT defaultScissorRect;

		GraphicsContext(Ptr<ResourcePool> resourcePool,
						Ptr<ConstantBufferPool> cbufferPool,
						Ptr<DynamicDescriptorHeap> dHeaps,
						Ptr<ID3D12GraphicsCommandList> cl,
						const D3D12_CPU_DESCRIPTOR_HANDLE & backbuffer,
						const D3D12_CPU_DESCRIPTOR_HANDLE & backbufferDepth,
						const D3D12_VIEWPORT & viewPort,
						const D3D12_RECT & scissorRect);

		virtual void SetStencilReference(uint8_t stencilValue) override;

		virtual void SetVertexBuffer(Ref<GpuResource> handle) override;

		virtual void SetIndexBuffer(Ref<GpuResource> handle) override;

		virtual void DrawIndexed(uint32_t indexCount) override;

		virtual void DrawIndexed(uint32_t indexCount, uint32_t vertexOffset) override;

		virtual void Draw(uint32_t vertexCount) override;

		virtual void Draw(uint32_t vertexCount, uint32_t vertexOffset) override;

		virtual void Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ) override;

		virtual void SetRootSignature(Ref<Netcode::RootSignature> rs) override;

		virtual void SetPipelineState(Ref<Netcode::PipelineState> pso) override;

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) override;

		virtual void ClearUnorderedAccessViewUint(Ref<GpuResource> handle, const UInt4 & values) override;

		virtual void ClearDepthOnly() override;
		virtual void ClearStencilOnly() override;
		virtual void ClearDepthStencil() override;

		virtual void ClearRenderTarget(uint8_t idx) override;
		virtual void ClearRenderTarget(uint8_t idx, const float * clearColor) override;

		virtual void SetStreamOutput(Ref<GpuResource> handle) override;

		virtual void SetStreamOutputFilledSize(Ref<GpuResource> handle, uint64_t byteOffset) override;
		virtual void ResetStreamOutput() override;

		virtual void SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) override;
		virtual void SetViewport(uint32_t width, uint32_t height) override;
		virtual void SetViewport() override;

		virtual Float4 GetViewport() override;

		virtual void SetScissorRect(int32_t left, int32_t right, int32_t top, int32_t bottom) override;
		virtual void SetScissorRect(uint32_t width, uint32_t height) override;
		virtual void SetScissorRect() override;

		virtual void SetRenderTargets(std::initializer_list<Ref<GpuResource>> handles, Ref<GpuResource> depthStencil) override;

		virtual void SetRenderTargets(std::nullptr_t rt, std::nullptr_t ds) override;

		virtual void SetRenderTargets(std::nullptr_t rt, Ref<Netcode::ResourceViews> ds) override;
		virtual void SetRenderTargets(Ref<Netcode::ResourceViews> rt, std::nullptr_t ds) override;
		virtual void SetRenderTargets(Ref<Netcode::ResourceViews> renderTargets, Ref<Netcode::ResourceViews> depthStencil) override;

		virtual void SetRenderTargets(std::nullptr_t rt, Ref<GpuResource> ds) override;
		virtual void SetRenderTargets(Ref<GpuResource> rt, std::nullptr_t ds) override;
		virtual void SetRenderTargets(Ref<GpuResource> renderTarget, Ref<GpuResource> depthStencil) override;

		virtual void SetShaderResources(int slot, std::initializer_list<Ref<GpuResource>> shaderResourceHandles) override;
		virtual void SetShaderResources(int slot, Ref<Netcode::ResourceViews> resourceView) override;
		virtual void SetShaderResources(int slot, Ref<Netcode::ResourceViews> resourceView, int descriptorOffset) override;
		virtual void SetShaderResources(int slot, Ref<GpuResource> nonTextureResource) override;

		virtual void SetRootConstants(int slot, const void * srcData, uint32_t numConstants) override;
		virtual void SetRootConstants(int slot, const void * srcData, uint32_t num32bitConstants, uint32_t offsetIn32BitConstants) override;
		virtual void SetConstantBuffer(int slot, Ref<GpuResource> cbufferHandle) override;

		virtual void SetConstants(int slot, uint64_t constantHandle) override;

		virtual uint64_t SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes) override;

		virtual void CopyBufferRegion(Ref<GpuResource> dstResource, Ref<GpuResource> srcResource, size_t sizeInBytes) override;

		virtual void CopyBufferRegion(Ref<GpuResource> dstResource, size_t dstOffset, Ref<GpuResource> srcResource, size_t srcOffset, size_t sizeInBytes) override;

		virtual void BeginPass() override;

		virtual void EndPass() override;

	};

}
