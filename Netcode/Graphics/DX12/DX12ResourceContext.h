#pragma once

#include "../GraphicsContexts.h"
#include "DX12ResourcePool.h"
#include "DX12DynamicDescriptorHeap.h"

namespace Netcode::Graphics::DX12 {

	class ResourceContext : public Netcode::Graphics::IResourceContext {
		ResourcePool * resources;
		com_ptr<ID3D12Device> device;
		RenderPassRef activeRenderPass;
	public:
		DynamicDescriptorHeap * descHeaps;
		D3D12_RECT backbufferExtents;

		void SetResourcePool(ResourcePool * pool) {
			resources = pool;
		}

		void SetDevice(com_ptr<ID3D12Device> dev) {
			device = std::move(dev);
		}

		virtual void UseComputeContext() override;
		virtual void UseGraphicsContext() override;

		virtual void Writes(uint64_t virtualDependency) override;
		virtual void Reads(uint64_t virtualDependency) override;

		virtual void Writes(GpuResourceRef resourceHandle) override;
		virtual void Reads(GpuResourceRef resourceHandle) override;

		virtual void SetRenderPass(RenderPassRef renderPass) override;
		virtual void ClearRenderPass() override;

		virtual GpuResourceRef CreateResource(const ResourceDesc & resource) override;

		virtual GpuResourceRef CreateReadbackBuffer(size_t size, ResourceType type, ResourceFlags flags) override;
		virtual GpuResourceRef CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) override;
		virtual GpuResourceRef CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags) override;

		virtual GpuResourceRef CreateTexture2D(const Image * images) override;
		virtual GpuResourceRef CreateTexture2D(const Image * images, ResourceType resourceType) override;
		virtual GpuResourceRef CreateTexture2D(const Image * images, uint32_t mipLevels) override;
		virtual GpuResourceRef CreateTexture2D(const Image * images, uint32_t mipLevels, ResourceType resourceType) override;
		virtual GpuResourceRef CreateTexture2D(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) override;
		virtual GpuResourceRef CreateTextureCube(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) override;

		virtual ResourceViewsRef CreateShaderResourceViews(uint32_t numDescriptors) override;
		virtual ResourceViewsRef CreateRenderTargetViews(uint32_t numDescriptors) override;
		virtual ResourceViewsRef CreateDepthStencilView() override;

		virtual GpuResourceRef CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) override;
		virtual GpuResourceRef CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual GpuResourceRef CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) override;
		virtual GpuResourceRef CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) override;
		virtual GpuResourceRef CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) override;
		virtual GpuResourceRef CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual GpuResourceRef CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) override;
		virtual GpuResourceRef CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType) override;

		virtual GpuResourceRef CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) override;
		virtual GpuResourceRef CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual GpuResourceRef CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) override;
		virtual GpuResourceRef CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) override;
		virtual GpuResourceRef CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) override;
		virtual GpuResourceRef CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual GpuResourceRef CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) override;
		virtual GpuResourceRef CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType) override;

		virtual GpuResourceRef CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState) override;
		virtual GpuResourceRef CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags) override;

		virtual GpuResourceRef CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState) override;
		virtual GpuResourceRef CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) override;

		virtual GpuResourceRef CreateConstantBuffer(size_t size) override;

		virtual void Readback(GpuResourceRef readbackResource, void * dstData, size_t dstDataSizeInBytes) override;
		virtual void Readback(GpuResourceRef readbackResource, void * dstData, size_t dstDataSizeInBytes, size_t srcOffsetInBytes) override;

		virtual void CopyConstants(GpuResourceRef uploadResource, const void * srcData, size_t srcDataSizeInBytes) override;
		virtual void CopyConstants(GpuResourceRef uploadResource, const void * srcData, size_t srcDataSizeInBytes, size_t dstOffsetInBytes) override;

		virtual void SetDebugName(GpuResourceRef resourceHandle, const wchar_t * name) override;
	};

	using DX12ResourceContext = Netcode::Graphics::DX12::ResourceContext;
	using DX12ResourceContextRef = std::shared_ptr<DX12ResourceContext>;

}
