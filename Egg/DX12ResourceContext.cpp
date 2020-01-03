#include "DX12ResourceContext.h"

namespace Egg::Graphics::DX12 {

	uint64_t ResourceContext::CreateResource(const ResourceDesc & resource)
	{
		return uint64_t();
	}

	uint64_t ResourceContext::CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags)
	{
		return uint64_t();
	}

	uint64_t ResourceContext::CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags)
	{
		return uint64_t();
	}

	uint64_t ResourceContext::CreateTexture2D()
	{
		return uint64_t();
	}

	uint64_t ResourceContext::CreateRenderTarget()
	{
		return uint64_t();
	}

	uint64_t ResourceContext::CreateDepthStencil()
	{
		return uint64_t();
	}

	const ResourceDesc & ResourceContext::QueryDesc(uint64_t handle)
	{
		return reinterpret_cast<GResource *>(handle)->desc;
	}

	void ResourceContext::CpuOnlyRenderPass()
	{
	}

	uint64_t ResourceContext::CreateConstantBuffer(size_t size)
	{
#undef GENERIC_READ
		ResourceDesc desc;
		desc.depth = 1;
		desc.dimension = ResourceDimension::BUFFER;
		desc.width = size;
		desc.mipLevels = 1;
		desc.format = DXGI_FORMAT_UNKNOWN;
		desc.flags = ResourceFlags::NONE;
		desc.type = ResourceType::PERMANENT_UPLOAD;
		desc.sizeInBytes = desc.width;
		desc.height = 1;
		desc.state = ResourceState::GENERIC_READ;
		return resources->CreateResource(desc);
	}

	void ResourceContext::CopyConstants(uint64_t cbufferHandle, const void * srcData, size_t srcDataSizeInBytes)
	{
	}

	uint64_t ResourceContext::CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState)
	{
		return CreateVertexBuffer(size, stride, type, initState, ResourceFlags::NONE);
	}

	uint64_t ResourceContext::CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags)
	{
		return resources->CreateStructuredBuffer(size, stride, type, initState, flags);
	}

	uint64_t ResourceContext::CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState)
	{
		return CreateIndexBuffer(size, format, type, initState, ResourceFlags::NONE);
	}

	uint64_t ResourceContext::CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags)
	{
		return resources->CreateTypedBuffer(size, format, type, initState, flags);
	}

}
