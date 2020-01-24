#include "DX12ResourcePool.h"

namespace Egg::Graphics::DX12 {

	uint64_t ResourcePool::CreateResource(const ResourceDesc & resource) {
		GResource res;
		res.desc = resource;
		res.resource = heapManager->CreateResource(res.desc);
		res.address = (res.desc.dimension == ResourceDimension::BUFFER) ? res.resource->GetGPUVirtualAddress() : 0;
		return reinterpret_cast<uint64_t>(&managedResources.emplace_back(res));
	}

	uint64_t ResourcePool::CreateStructuredBuffer(size_t sizeInBytes, UINT strideInBytes, ResourceType type, ResourceState initialState, ResourceFlags flags) {
		ResourceDesc rDesc;
		rDesc.type = type;
		rDesc.dimension = ResourceDimension::BUFFER;
		rDesc.height = 1;
		rDesc.mipLevels = 1;
		rDesc.sizeInBytes = rDesc.width = sizeInBytes;
		rDesc.strideInBytes = strideInBytes;
		rDesc.format = DXGI_FORMAT_UNKNOWN;
		rDesc.depth = 1;
		rDesc.flags = flags;
		rDesc.state = initialState;

		return CreateResource(rDesc);
	}

	uint64_t ResourcePool::CreateTypedBuffer(size_t sizeInBytes, DXGI_FORMAT format, ResourceType type, ResourceState initialState, ResourceFlags flags) {
		ResourceDesc rDesc;
		rDesc.type = type;
		rDesc.dimension = ResourceDimension::BUFFER;
		rDesc.height = 1;
		rDesc.mipLevels = 1;
		rDesc.sizeInBytes = rDesc.width = sizeInBytes;
		rDesc.strideInBytes = static_cast<UINT>(DirectX::BitsPerPixel(format)) / 8U;
		rDesc.format = format;
		rDesc.depth = 1;
		rDesc.flags = flags;
		rDesc.state = initialState;

		return CreateResource(rDesc);
	}

	uint64_t ResourcePool::CreateDepthBuffer(ResourceType type, UINT width, UINT height, DXGI_FORMAT format, ResourceState initialState, ResourceFlags optFlags) {
		ResourceDesc rDesc;
		rDesc.type = type;
		rDesc.dimension = ResourceDimension::TEXTURE2D;
		rDesc.height = height;
		rDesc.width = static_cast<UINT64>(width);
		rDesc.depth = 1;
		rDesc.mipLevels = 1;
		rDesc.strideInBytes = static_cast<UINT>(DirectX::BitsPerPixel(format)) / 8U;
		rDesc.sizeInBytes = 0;
		rDesc.format = format;
		rDesc.flags = optFlags;
		rDesc.state = initialState;

		bool isFormatValid = (format == DXGI_FORMAT_D16_UNORM ||
			format == DXGI_FORMAT_D32_FLOAT ||
			format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
			format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT);

		ASSERT(isFormatValid, "Depth buffer format is invalid");

		bool hasDepthFlag = (GetNativeFlags(optFlags) & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0;

		ASSERT(hasDepthFlag, "Depth buffer must have D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL flag set");

		bool isTypeValid = (type == ResourceType::PERMANENT_DEFAULT || type == ResourceType::TRANSIENT_DEFAULT);

		ASSERT(isTypeValid, "Depth buffer must be present in the default heap");

		return CreateResource(rDesc);
	}

	const ResourceDesc & ResourcePool::GetResourceDesc(uint64_t handle) {
		return reinterpret_cast<GResource *>(handle)->desc;
	}

	const GResource & ResourcePool::GetNativeResource(uint64_t handle) {
		return *(reinterpret_cast<GResource *>(handle));
	}

	void ResourcePool::ReleaseResource(uint64_t handle) {
		GResource * ptr = reinterpret_cast<GResource *>(handle);

		heapManager->ReleaseResource(ptr->resource);

		for(decltype(managedResources)::iterator it = managedResources.begin(); it != managedResources.end(); ++it) {
			if((&(*it)) == ptr) {
				managedResources.erase(it);
				return;
			}
		}

	}
	void ResourcePool::ReleaseTransients()
	{
		for(const auto & i : managedResources) {
			if(i.desc.type == ResourceType::TRANSIENT_DEFAULT ||
				i.desc.type == ResourceType::TRANSIENT_UPLOAD ||
				i.desc.type == ResourceType::TRANSIENT_READBACK) {
				heapManager->ReleaseResource(i.resource);
			}
		}
		managedResources.remove_if([](const GResource & gres) ->bool {
			return gres.desc.type == ResourceType::TRANSIENT_DEFAULT ||
				gres.desc.type == ResourceType::TRANSIENT_UPLOAD ||
				gres.desc.type == ResourceType::TRANSIENT_READBACK;
		});
	}
}
