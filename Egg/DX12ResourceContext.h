#pragma once

#include "GraphicsContexts.h"
#include "DX12ResourcePool.h"

namespace Egg::Graphics::DX12 {

	class ResourceContext : public Egg::Graphics::IResourceContext {
		
		ResourcePool * resources;
		

	public:

		void SetResourcePool(ResourcePool * pool) {
			resources = pool;
		}

		// Inherited via IResourceContext
		virtual uint64_t CreateResource(const ResourceDesc & resource) override;
		virtual uint64_t CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) override;
		virtual uint64_t CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags) override;
		virtual uint64_t CreateTexture2D() override;
		virtual uint64_t CreateRenderTarget() override;
		virtual uint64_t CreateDepthStencil() override;
		virtual const ResourceDesc & QueryDesc(uint64_t handle) override;
		virtual void CpuOnlyRenderPass() override;
		virtual uint64_t CreateConstantBuffer(size_t size) override;
		virtual void CopyConstants(uint64_t cbufferHandle, const void * srcData, size_t srcDataSizeInBytes) override;
		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState) override;
		virtual uint64_t CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags) override;
		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState) override;
		virtual uint64_t CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags) override;
	};

}
