#pragma once

#include <Netcode/Graphics/GraphicsContexts.h>
#include "DX12Decl.h"
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	class ResourcePool;
	class DynamicDescriptorHeap;

	class ResourceContext : public Netcode::Graphics::IResourceContext {
		Ptr<ResourcePool> resources;
		com_ptr<ID3D12Device> device;
		Ref<RenderPass> activeRenderPass;
	public:
		Ptr<DynamicDescriptorHeap> descHeaps;
		D3D12_RECT backbufferExtents;

		void SetResourcePool(ResourcePool * pool) {
			resources = pool;
		}

		void SetDevice(com_ptr<ID3D12Device> dev) {
			device = std::move(dev);
		}

		virtual void UseComputeContext() override;
		virtual void UseGraphicsContext() override;

		virtual void Writes(uintptr_t virtualDependency) override;
		virtual void Reads(uintptr_t virtualDependency) override;

		virtual void Writes(Ptr<GpuResource> resourceHandle) override;
		virtual void Reads(Ptr<GpuResource> resourceHandle) override;

		virtual void SetRenderPass(Ref<Netcode::RenderPass> renderPass) override;
		virtual void ClearRenderPass() override;

		virtual Ref<GpuResource> CreateResource(const ResourceDesc & resource) override;

		virtual Ref<GpuResource> CreateReadbackBuffer(size_t size, ResourceType type, ResourceFlags flags) override;
		virtual Ref<GpuResource> CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) override;
		virtual Ref<GpuResource> CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags) override;

		virtual Ref<GpuResource> CreateTexture2D(const Image * images) override;
		virtual Ref<GpuResource> CreateTexture2D(const Image * images, ResourceType resourceType) override;
		virtual Ref<GpuResource> CreateTexture2D(const Image * images, uint32_t mipLevels) override;
		virtual Ref<GpuResource> CreateTexture2D(const Image * images, uint32_t mipLevels, ResourceType resourceType) override;
		virtual Ref<GpuResource> CreateTexture2D(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) override;
		virtual Ref<GpuResource> CreateTextureCube(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) override;

		virtual Ref<Netcode::ResourceViews> CreateShaderResourceViews(uint32_t numDescriptors) override;
		virtual Ref<Netcode::ResourceViews> CreateRenderTargetViews(uint32_t numDescriptors) override;
		virtual Ref<Netcode::ResourceViews> CreateDepthStencilView() override;

		virtual Ref<GpuResource> CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) override;
		virtual Ref<GpuResource> CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual Ref<GpuResource> CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) override;
		virtual Ref<GpuResource> CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) override;
		virtual Ref<GpuResource> CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) override;
		virtual Ref<GpuResource> CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual Ref<GpuResource> CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) override;
		virtual Ref<GpuResource> CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType) override;

		virtual Ref<GpuResource> CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) override;
		virtual Ref<GpuResource> CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual Ref<GpuResource> CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) override;
		virtual Ref<GpuResource> CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) override;
		virtual Ref<GpuResource> CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) override;
		virtual Ref<GpuResource> CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual Ref<GpuResource> CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) override;
		virtual Ref<GpuResource> CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType) override;

		virtual Ref<GpuResource> CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState) override;
		virtual Ref<GpuResource> CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags) override;

		virtual Ref<GpuResource> CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState) override;
		virtual Ref<GpuResource> CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) override;

		virtual Ref<GpuResource> CreateConstantBuffer(size_t size) override;

		virtual void Readback(Ref<GpuResource> readbackResource, void * dstData, size_t dstDataSizeInBytes) override;
		virtual void Readback(Ref<GpuResource> readbackResource, void * dstData, size_t dstDataSizeInBytes, size_t srcOffsetInBytes) override;

		virtual void CopyConstants(Ref<GpuResource> uploadResource, const void * srcData, size_t srcDataSizeInBytes) override;
		virtual void CopyConstants(Ref<GpuResource> uploadResource, const void * srcData, size_t srcDataSizeInBytes, size_t dstOffsetInBytes) override;

		virtual void SetDebugName(Ref<GpuResource> resourceHandle, const wchar_t * name) override;

		virtual Ref<Netcode::Graphics::UploadBatch> CreateUploadBatch() override;
	};

	using DX12ResourceContext = Netcode::Graphics::DX12::ResourceContext;
	using DX12ResourceContextRef = Ref<DX12ResourceContext>;

}
