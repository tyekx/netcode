#include "DX12ResourceContext.h"
#include <memory>

namespace Netcode::Graphics::DX12 {

	void ResourceContext::UseComputeContext()
	{
		if(activeRenderPass != nullptr) {
			activeRenderPass->Type(Netcode::RenderPassType::COMPUTE);
		}
	}

	void ResourceContext::UseGraphicsContext()
	{
		if(activeRenderPass != nullptr) {
			activeRenderPass->Type(Netcode::RenderPassType::DIRECT);
		}
	}

	void ResourceContext::Writes(GpuResourceRef resourceHandle)
	{
		if(activeRenderPass != nullptr) {
			activeRenderPass->WritesResource(reinterpret_cast<uint64_t>(resourceHandle.get()));
		}
	}

	void ResourceContext::Reads(GpuResourceRef resourceHandle)
	{
		if(activeRenderPass != nullptr) {
			activeRenderPass->ReadsResource(reinterpret_cast<uint64_t>(resourceHandle.get()));
		}
	}

	void ResourceContext::SetRenderPass(RenderPassRef renderPass)
	{
		activeRenderPass = std::move(renderPass);
	}

	void ResourceContext::ClearRenderPass()
	{
		activeRenderPass.reset();
	}

	GpuResourceRef ResourceContext::CreateResource(const ResourceDesc & resource)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
		return nullptr;
	}

	GpuResourceRef ResourceContext::CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags)
	{
		return resources->CreateTypedBuffer(size, format, type, initState, flags);
	}

	GpuResourceRef ResourceContext::CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceState initState, ResourceFlags flags)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
		return nullptr;
	}

	GpuResourceRef ResourceContext::CreateTexture2D(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags)
	{
		ASSERT(resourceType == ResourceType::PERMANENT_DEFAULT || resourceType == ResourceType::TRANSIENT_DEFAULT, "Texture2D must be in default heap");
		ResourceDesc desc;
		desc.depth = 1;
		desc.dimension = ResourceDimension::TEXTURE2D;
		desc.width = width;
		desc.mipLevels = 1;
		desc.format = format;
		desc.flags = flags;
		desc.type = resourceType;
		desc.sizeInBytes = 0;
		desc.strideInBytes = static_cast<uint32_t>(DirectX::BitsPerPixel(format) / 8U);
		desc.height = height;
		desc.state = initialState;
		ZeroMemory(&desc.clearValue, sizeof(ClearValue));
		return resources->CreateResource(desc);
	}

	GpuResourceRef ResourceContext::CreateTextureCube(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initialState, ResourceFlags flags)
	{
		ASSERT(resourceType == ResourceType::PERMANENT_DEFAULT || resourceType == ResourceType::TRANSIENT_DEFAULT, "TextureCube must be in default heap");
		ResourceDesc desc;
		desc.depth = 6;
		desc.dimension = ResourceDimension::TEXTURE2D;
		desc.width = width;
		desc.mipLevels = 1;
		desc.format = format;
		desc.flags = flags;
		desc.type = resourceType;
		desc.sizeInBytes = 0;
		desc.strideInBytes = static_cast<uint32_t>(DirectX::BitsPerPixel(format) / 8U);
		desc.height = height;
		desc.state = initialState;
		ZeroMemory(&desc.clearValue, sizeof(ClearValue));
		return resources->CreateResource(desc);
	}

	GpuResourceRef ResourceContext::CreateTexture2D(const Image * images)
	{
		return CreateTexture2D(images, ResourceType::PERMANENT_DEFAULT);
	}

	GpuResourceRef ResourceContext::CreateTexture2D(const Image * images, ResourceType resourceType)
	{
		return CreateTexture2D(static_cast<uint32_t>(images->width), static_cast<uint32_t>(images->height), images->format, resourceType, ResourceState::COPY_DEST, ResourceFlags::NONE);
	}

	GpuResourceRef ResourceContext::CreateTexture2D(const Image * images, uint32_t mipLevels)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
		return nullptr;
	}

	GpuResourceRef ResourceContext::CreateTexture2D(const Image * images, uint32_t mipLevels, ResourceType resourceType)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
		return nullptr;
	}

	ResourceViewsRef ResourceContext::CreateShaderResourceViews(uint32_t numDescriptors)
	{
		return descHeaps->CreatePermanentSRV(numDescriptors);
	}

	ResourceViewsRef ResourceContext::CreateRenderTargetViews(uint32_t numDescriptors)
	{
		return descHeaps->CreatePermanentRTV(numDescriptors);
	}

	ResourceViewsRef ResourceContext::CreateDepthStencilView()
	{
		return descHeaps->CreatePermanentDSV();
	}

	void ResourceContext::SetDebugName(GpuResourceRef resourceHandle, const wchar_t * name)
	{
		std::dynamic_pointer_cast<DX12Resource>(resourceHandle)->resource->SetName(name);
	}

	GpuResourceRef ResourceContext::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor)
	{
		ASSERT(resourceType == ResourceType::PERMANENT_DEFAULT || resourceType == ResourceType::TRANSIENT_DEFAULT, "Texture2D must be in default heap");
		ResourceDesc desc;
		desc.depth = 1;
		desc.dimension = ResourceDimension::TEXTURE2D;
		desc.width = width;
		desc.mipLevels = 1;
		desc.format = format;
		desc.flags = ResourceFlags::ALLOW_RENDER_TARGET;
		desc.type = resourceType;
		desc.sizeInBytes = 0;
		desc.strideInBytes = static_cast<uint32_t>(DirectX::BitsPerPixel(format) / 8U);
		desc.height = height;
		desc.state = initState;
		desc.clearValue.Color[0] = clearColor.x;
		desc.clearValue.Color[1] = clearColor.y;
		desc.clearValue.Color[2] = clearColor.z;
		desc.clearValue.Color[3] = clearColor.w;
		return resources->CreateResource(desc);
	}

	GpuResourceRef ResourceContext::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor)
	{
		return CreateRenderTarget(width, height, format, resourceType, ResourceState::RENDER_TARGET, clearColor);
	}

	GpuResourceRef ResourceContext::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState)
	{
		return CreateRenderTarget(width, height, format, resourceType, initState, DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f });
	}

	GpuResourceRef ResourceContext::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType)
	{
		return CreateRenderTarget(width, height, format, resourceType, ResourceState::RENDER_TARGET);
	}

	GpuResourceRef ResourceContext::CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, const DirectX::XMFLOAT4 & clearColor)
	{
		return CreateRenderTarget(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, initState, clearColor);
	}

	GpuResourceRef ResourceContext::CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor)
	{
		return CreateRenderTarget(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, ResourceState::RENDER_TARGET, clearColor);
	}

	GpuResourceRef ResourceContext::CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState)
	{
		return CreateRenderTarget(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, initState, DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f });
	}

	GpuResourceRef ResourceContext::CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType)
	{
		return CreateRenderTarget(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, ResourceState::RENDER_TARGET, DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f });
	}

	GpuResourceRef ResourceContext::CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil)
	{
		ASSERT(resourceType == ResourceType::PERMANENT_DEFAULT || resourceType == ResourceType::TRANSIENT_DEFAULT, "Texture2D must be in default heap");
		ResourceDesc desc;
		desc.depth = 1;
		desc.dimension = ResourceDimension::TEXTURE2D;
		desc.width = width;
		desc.mipLevels = 1;
		desc.format = format;
		desc.flags = ResourceFlags::ALLOW_DEPTH_STENCIL;
		desc.type = resourceType;
		desc.sizeInBytes = 0;
		desc.strideInBytes = static_cast<uint32_t>(DirectX::BitsPerPixel(format) / 8U);
		desc.height = height;
		desc.state = initState;
		desc.clearValue.DepthStencil.Depth = clearDepth;
		desc.clearValue.DepthStencil.Stencil = clearStencil;
		return resources->CreateResource(desc);
	}

	GpuResourceRef ResourceContext::CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil)
	{
		return CreateDepthStencil(width, height, format, resourceType, ResourceState::DEPTH_WRITE, clearDepth, clearStencil);
	}

	GpuResourceRef ResourceContext::CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceState initState)
	{
		return CreateDepthStencil(width, height, format, resourceType, initState, 1.0f, 0);
	}

	GpuResourceRef ResourceContext::CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType)
	{
		return CreateDepthStencil(width, height, format, resourceType, ResourceState::DEPTH_WRITE, 1.0f, 0);
	}

	GpuResourceRef ResourceContext::CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState, float clearDepth, uint8_t clearStencil)
	{
		return CreateDepthStencil(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, initState, clearDepth, clearStencil);
	}

	GpuResourceRef ResourceContext::CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil)
	{
		return CreateDepthStencil(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, ResourceState::DEPTH_WRITE, clearDepth, clearStencil);
	}

	GpuResourceRef ResourceContext::CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceState initState)
	{
		return CreateDepthStencil(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, initState, 1.0f, 0);
	}

	GpuResourceRef ResourceContext::CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType)
	{
		return CreateDepthStencil(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, ResourceState::DEPTH_WRITE, 1.0f, 0);
	}

	GpuResourceRef ResourceContext::CreateConstantBuffer(size_t size)
	{
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
		desc.state = ResourceState::ANY_READ;
		return resources->CreateResource(desc);
	}

	void ResourceContext::CopyConstants(GpuResourceRef uploadResource, const void * srcData, size_t srcDataSizeInBytes, size_t dstOffsetInBytes) {
		ID3D12Resource * resource = std::dynamic_pointer_cast<DX12Resource>(uploadResource)->resource.Get();

		uint8_t * mappedPtr;
		const CD3DX12_RANGE readRange{ 0,0 };
		const CD3DX12_RANGE writtenRange{ dstOffsetInBytes, srcDataSizeInBytes };

		DX_API("Failed to map ptr")
			resource->Map(0, &readRange, reinterpret_cast<void **>(&mappedPtr));

		memcpy(mappedPtr + dstOffsetInBytes, srcData, srcDataSizeInBytes);

		resource->Unmap(0, &writtenRange);
	}

	void ResourceContext::Readback(GpuResourceRef readbackResource, void * dstData, size_t dstDataSizeInBytes)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ResourceContext::Readback(GpuResourceRef readbackResource, void * dstData, size_t dstDataSizeInBytes, size_t dstOffsetInBytes)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ResourceContext::CopyConstants(GpuResourceRef uploadResource, const void * srcData, size_t srcDataSizeInBytes)
	{
		CopyConstants(uploadResource, srcData, srcDataSizeInBytes, 0);
	}

	GpuResourceRef ResourceContext::CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState)
	{
		return CreateVertexBuffer(size, stride, type, initState, ResourceFlags::NONE);
	}

	GpuResourceRef ResourceContext::CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceState initState, ResourceFlags flags)
	{
		return resources->CreateStructuredBuffer(size, stride, type, initState, flags);
	}

	GpuResourceRef ResourceContext::CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState)
	{
		return CreateIndexBuffer(size, format, type, initState, ResourceFlags::NONE);
	}

	GpuResourceRef ResourceContext::CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceState initState, ResourceFlags flags)
	{
		return resources->CreateTypedBuffer(size, format, type, initState, flags);
	}

}
