#pragma once

#include "GraphicsContexts.h"
#include "DX12ResourcePool.h"
#include "DX12DynamicDescriptorHeap.h"

namespace Egg::Graphics::DX12 {

	class ResourceContext : public Egg::Graphics::IResourceContext {

		ResourcePool * resources;
		com_ptr<ID3D12Device> device;

	public:
		DynamicDescriptorHeap * descHeaps;
		D3D12_RECT backbufferExtents;

		void SetResourcePool(ResourcePool * pool) {
			resources = pool;
		}

		void SetDevice(com_ptr<ID3D12Device> dev) {
			device = std::move(dev);
		}

		// Inherited via IResourceContext
		virtual uint64_t CreateResource(const ResourceDesc & resource) override;
		virtual uint64_t CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) override;
		virtual uint64_t CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags) override;
		virtual uint64_t CreateTexture2D(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags) override;

		virtual ResourceViewsRef CreateShaderResourceViews(uint32_t numDescriptors) override;
		virtual ResourceViewsRef CreateRenderTargetViews(uint32_t numDescriptors) override;
		virtual ResourceViewsRef CreateDepthStencilView() override;

		virtual void SetDebugName(uint64_t resourceHandle, const wchar_t * name) override;

		virtual uint64_t CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) override;
		virtual uint64_t CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual uint64_t CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) override;
		virtual uint64_t CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) override;
		virtual uint64_t CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor) override;
		virtual uint64_t CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual uint64_t CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor) override;
		virtual uint64_t CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType) override;

		virtual uint64_t CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) override;
		virtual uint64_t CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual uint64_t CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) override;
		virtual uint64_t CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType) override;
		virtual uint64_t CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil) override;
		virtual uint64_t CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState) override;
		virtual uint64_t CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil) override;
		virtual uint64_t CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType) override;

		virtual const ResourceDesc & QueryDesc(uint64_t handle) override;
		virtual void ReleaseResource(uint64_t handle) override;
		virtual uint64_t CreateConstantBuffer(size_t size) override;
		virtual void CopyConstants(uint64_t cbufferHandle, const void * srcData, size_t srcDataSizeInBytes) override;
		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState) override;
		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags) override;
		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState) override;
		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) override;
	};

}
