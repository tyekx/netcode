#include "DX12ResourceContext.h"
#include <Netcode/Common.h>
#include <Netcode/HandleTypes.h>
#include <Netcode/Graphics/ResourceDesc.h>
#include "DX12ResourcePool.h"
#include "DX12DynamicDescriptorHeap.h"
#include "DX12UploadBatch.h"
#include "DX12RenderPass.h"
#include "DX12ResourceViews.h"
#include "DX12Resource.h"
#include <memory>
#include <DirectXTex.h>

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
			activeRenderPass->Type(Netcode::RenderPassType::GRAPHICS);
		}
	}

	void ResourceContext::Writes(uintptr_t virtualDependency)
	{
		if(activeRenderPass != nullptr) {
			activeRenderPass->WritesResource(virtualDependency);
		}
	}

	void ResourceContext::Reads(uintptr_t virtualDependency)
	{
		if(activeRenderPass != nullptr) {
			activeRenderPass->ReadsResource(virtualDependency);
		}
	}

	void ResourceContext::Writes(Ptr<GpuResource> resourceHandle)
	{
		if(activeRenderPass != nullptr) {
			activeRenderPass->WritesResource(reinterpret_cast<uintptr_t>(resourceHandle));
		}
	}

	void ResourceContext::Reads(Ptr<GpuResource> resourceHandle)
	{
		if(activeRenderPass != nullptr) {
			activeRenderPass->ReadsResource(reinterpret_cast<uintptr_t>(resourceHandle));
		}
	}

	void ResourceContext::SetRenderPass(Ref<Netcode::RenderPass> renderPass)
	{
		activeRenderPass = std::move(renderPass);
	}

	void ResourceContext::ClearRenderPass()
	{
		activeRenderPass.reset();
	}

	Ref<GpuResource> ResourceContext::CreateResource(const ResourceDesc & resource)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
		return nullptr;
	}

	Ref<GpuResource> ResourceContext::CreateReadbackBuffer(size_t size, ResourceType type, ResourceFlags flags)
	{
		ResourceDesc desc;
		desc.depth = 1;
		desc.dimension = ResourceDimension::BUFFER;
		desc.width = size;
		desc.mipLevels = 1;
		desc.format = DXGI_FORMAT_UNKNOWN;
		desc.flags = flags;
		desc.type = type;
		desc.sizeInBytes = desc.width;
		desc.height = 1;
		desc.state = ResourceState::COPY_DEST;
		return resources->CreateResource(desc);
	}

	Ref<GpuResource> ResourceContext::CreateTypedBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceStates initState, ResourceFlags flags)
	{
		return resources->CreateTypedBuffer(size, format, type, initState, flags);
	}

	Ref<GpuResource> ResourceContext::CreateStructuredBuffer(size_t size, uint32_t stride, ResourceType type, ResourceStates initState, ResourceFlags flags)
	{
		ResourceDesc desc;
		desc.depth = 1;
		desc.dimension = ResourceDimension::BUFFER;
		desc.width = size;
		desc.mipLevels = 1;
		desc.strideInBytes = stride;
		desc.format = DXGI_FORMAT_UNKNOWN;
		desc.flags = flags;
		desc.type = type;
		desc.sizeInBytes = desc.width;
		desc.height = 1;
		desc.state = initState;
		return resources->CreateResource(desc);
	}

	Ref<GpuResource> ResourceContext::CreateTexture2D(uint32_t width, uint32_t height, uint16_t mipLevels, DXGI_FORMAT format, ResourceType resourceType, ResourceStates initialState, ResourceFlags flags)
	{
		ASSERT(resourceType == ResourceType::PERMANENT_DEFAULT || resourceType == ResourceType::TRANSIENT_DEFAULT, "Texture2D must be in default heap");
		ResourceDesc desc;
		desc.depth = 1;
		desc.dimension = ResourceDimension::TEXTURE2D;
		desc.width = width;
		desc.mipLevels = mipLevels;
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

	Ref<GpuResource> ResourceContext::CreateTextureCube(uint32_t width, uint32_t height, uint16_t mipLevels, DXGI_FORMAT format, ResourceType resourceType, ResourceStates initialState, ResourceFlags flags)
	{
		ASSERT(resourceType == ResourceType::PERMANENT_DEFAULT || resourceType == ResourceType::TRANSIENT_DEFAULT, "TextureCube must be in default heap");
		ResourceDesc desc;
		desc.depth = 6;
		desc.dimension = ResourceDimension::TEXTURE2D;
		desc.width = width;
		desc.mipLevels = mipLevels;
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

	Ref<GpuResource> ResourceContext::CreateTexture2D(const Image * images)
	{
		return CreateTexture2D(images, ResourceType::PERMANENT_DEFAULT);
	}

	Ref<GpuResource> ResourceContext::CreateTexture2D(const Image * images, ResourceType resourceType)
	{
		return CreateTexture2D(static_cast<uint32_t>(images->width), static_cast<uint32_t>(images->height), 1, images->format, resourceType, ResourceState::COPY_DEST, ResourceFlags::NONE);
	}

	Ref<GpuResource> ResourceContext::CreateTexture2D(const Image * images, uint32_t mipLevels)
	{
		return CreateTexture2D(static_cast<uint32_t>(images->width), static_cast<uint32_t>(images->height), mipLevels, images->format, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);
	}

	Ref<GpuResource> ResourceContext::CreateTexture2D(const Image * images, uint32_t mipLevels, ResourceType resourceType)
	{
		return CreateTexture2D(static_cast<uint32_t>(images->width), static_cast<uint32_t>(images->height), mipLevels, images->format, resourceType, ResourceState::COPY_DEST, ResourceFlags::NONE);
	}

	Ref<Netcode::ResourceViews> ResourceContext::CreateShaderResourceViews(uint32_t numDescriptors)
	{
		return descHeaps->CreatePermanentSRV(numDescriptors);
	}

	Ref<Netcode::ResourceViews> ResourceContext::CreateRenderTargetViews(uint32_t numDescriptors)
	{
		return descHeaps->CreatePermanentRTV(numDescriptors);
	}

	Ref<Netcode::ResourceViews> ResourceContext::CreateDepthStencilView()
	{
		return descHeaps->CreatePermanentDSV();
	}

	void ResourceContext::SetDebugName(Ref<GpuResource> resourceHandle, const wchar_t * name)
	{
		std::dynamic_pointer_cast<DX12::Resource>(resourceHandle)->resource->SetName(name);
	}

	Ref<Netcode::Graphics::UploadBatch> ResourceContext::CreateUploadBatch() {
		return std::make_shared<DX12::UploadBatchImpl>();
	}

	Ref<GpuResource> ResourceContext::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceStates initState, const DirectX::XMFLOAT4 & clearColor)
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

	Ref<GpuResource> ResourceContext::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor)
	{
		return CreateRenderTarget(width, height, format, resourceType, ResourceState::RENDER_TARGET, clearColor);
	}

	Ref<GpuResource> ResourceContext::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceStates initState)
	{
		return CreateRenderTarget(width, height, format, resourceType, initState, DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f });
	}

	Ref<GpuResource> ResourceContext::CreateRenderTarget(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType)
	{
		return CreateRenderTarget(width, height, format, resourceType, ResourceState::RENDER_TARGET);
	}

	Ref<GpuResource> ResourceContext::CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceStates initState, const DirectX::XMFLOAT4 & clearColor)
	{
		return CreateRenderTarget(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, initState, clearColor);
	}

	Ref<GpuResource> ResourceContext::CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, const DirectX::XMFLOAT4 & clearColor)
	{
		return CreateRenderTarget(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, ResourceState::RENDER_TARGET, clearColor);
	}

	Ref<GpuResource> ResourceContext::CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType, ResourceStates initState)
	{
		return CreateRenderTarget(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, initState, DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f });
	}

	Ref<GpuResource> ResourceContext::CreateRenderTarget(DXGI_FORMAT format, ResourceType resourceType)
	{
		return CreateRenderTarget(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, ResourceState::RENDER_TARGET, DirectX::XMFLOAT4{ 0.0f, 0.0f, 0.0f, 0.0f });
	}

	Ref<GpuResource> ResourceContext::CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceStates initState, float clearDepth, uint8_t clearStencil)
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

	Ref<GpuResource> ResourceContext::CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil)
	{
		return CreateDepthStencil(width, height, format, resourceType, ResourceState::DEPTH_WRITE, clearDepth, clearStencil);
	}

	Ref<GpuResource> ResourceContext::CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType, ResourceStates initState)
	{
		return CreateDepthStencil(width, height, format, resourceType, initState, 1.0f, 0);
	}

	Ref<GpuResource> ResourceContext::CreateDepthStencil(uint32_t width, uint32_t height, DXGI_FORMAT format, ResourceType resourceType)
	{
		return CreateDepthStencil(width, height, format, resourceType, ResourceState::DEPTH_WRITE, 1.0f, 0);
	}

	Ref<GpuResource> ResourceContext::CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceStates initState, float clearDepth, uint8_t clearStencil)
	{
		return CreateDepthStencil(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, initState, clearDepth, clearStencil);
	}

	Ref<GpuResource> ResourceContext::CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, float clearDepth, uint8_t clearStencil)
	{
		return CreateDepthStencil(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, ResourceState::DEPTH_WRITE, clearDepth, clearStencil);
	}

	Ref<GpuResource> ResourceContext::CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType, ResourceStates initState)
	{
		return CreateDepthStencil(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, initState, 1.0f, 0);
	}

	Ref<GpuResource> ResourceContext::CreateDepthStencil(DXGI_FORMAT format, ResourceType resourceType)
	{
		return CreateDepthStencil(backbufferExtents.right, backbufferExtents.bottom, format, resourceType, ResourceState::DEPTH_WRITE, 1.0f, 0);
	}

	Ref<GpuResource> ResourceContext::CreateConstantBuffer(size_t size)
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

	void ResourceContext::CopyConstants(Ref<GpuResource> uploadResource, const void * srcData, size_t srcDataSizeInBytes, size_t dstOffsetInBytes) {
		ID3D12Resource * resource = static_cast<DX12::Resource*>(uploadResource.get())->resource.Get();

		uint8_t * mappedPtr;
		const CD3DX12_RANGE readRange{ 0,0 };
		const CD3DX12_RANGE writtenRange{ dstOffsetInBytes, srcDataSizeInBytes };

		DX_API("Failed to map ptr")
			resource->Map(0, &readRange, reinterpret_cast<void **>(&mappedPtr));

		memcpy(mappedPtr + dstOffsetInBytes, srcData, srcDataSizeInBytes);

		resource->Unmap(0, &writtenRange);
	}

	void ResourceContext::Readback(Ref<GpuResource> readbackResource, void * dstData, size_t dstDataSizeInBytes)
	{
		Readback(readbackResource, dstData, dstDataSizeInBytes, 0);
	}

	void ResourceContext::Readback(Ref<GpuResource> readbackResource, void * dstData, size_t dstDataSizeInBytes, size_t srcOffsetInBytes)
	{
		ID3D12Resource * resource = std::dynamic_pointer_cast<DX12::Resource>(readbackResource)->resource.Get();

		uint8_t * mappedPtr;
		const CD3DX12_RANGE writtenRange{ 0,0 };
		const CD3DX12_RANGE readRange{ srcOffsetInBytes, dstDataSizeInBytes };

		DX_API("Failed to map ptr")
			resource->Map(0, &readRange, reinterpret_cast<void **>(&mappedPtr));

		memcpy(dstData, mappedPtr + srcOffsetInBytes, dstDataSizeInBytes);

		resource->Unmap(0, &writtenRange);
	}

	void ResourceContext::CopyConstants(Ref<GpuResource> uploadResource, const void * srcData, size_t srcDataSizeInBytes)
	{
		CopyConstants(uploadResource, srcData, srcDataSizeInBytes, 0);
	}

	Ref<GpuResource> ResourceContext::CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceStates initState)
	{
		return CreateVertexBuffer(size, stride, type, initState, ResourceFlags::NONE);
	}

	Ref<GpuResource> ResourceContext::CreateVertexBuffer(size_t size, unsigned int stride, ResourceType type, ResourceStates initState, ResourceFlags flags)
	{
		return resources->CreateStructuredBuffer(size, stride, type, initState, flags);
	}

	Ref<GpuResource> ResourceContext::CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceStates initState)
	{
		return CreateIndexBuffer(size, format, type, initState, ResourceFlags::NONE);
	}

	Ref<GpuResource> ResourceContext::CreateIndexBuffer(size_t size, DXGI_FORMAT format, ResourceType type, ResourceStates initState, ResourceFlags flags)
	{
		return resources->CreateTypedBuffer(size, format, type, initState, flags);
	}

}
