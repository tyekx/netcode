#include "DX12ResourcePool.h"

namespace Egg::Graphics::DX12 {

	ResourcePool::ResourcePool() : defaultPermanentHeaps{ D3D12_HEAP_TYPE_DEFAULT },
		uploadPermanentHeaps{ D3D12_HEAP_TYPE_UPLOAD },
		readbackPermanentHeaps{ D3D12_HEAP_TYPE_READBACK },
		defaultTransientHeaps{ D3D12_HEAP_TYPE_DEFAULT },
		uploadTransientHeaps{ D3D12_HEAP_TYPE_UPLOAD },
		readbackTransientHeaps{ D3D12_HEAP_TYPE_READBACK },
		totalSizeInBytes{ },
		freeSizeInBytes{},
		device{ nullptr },
		managedResources{} { }

	ResourcePool::HeapCollectionType & ResourcePool::GetCollection(ResourceType resType) {

		switch(resType) {
			case ResourceType::PERMANENT_DEFAULT:
				return defaultPermanentHeaps;
			case ResourceType::PERMANENT_UPLOAD:
				return uploadPermanentHeaps;
			case ResourceType::PERMANENT_READBACK:
				return readbackPermanentHeaps;
			case ResourceType::TRANSIENT_DEFAULT:
				return defaultTransientHeaps;
			case ResourceType::TRANSIENT_UPLOAD:
				return uploadTransientHeaps;
			case ResourceType::TRANSIENT_READBACK:
				return readbackTransientHeaps;
		}

		throw std::exception("Unexpected control flow");
	}

	void ResourcePool::SetDevice(ID3D12Device * device) {
		this->device = device;
	}

	uint64_t ResourcePool::CreateResource(const ResourceDesc & resource) {
		uint64_t idx = 0;

		HeapCollectionType & heapCollection = GetCollection(resource.type);

		D3D12_RESOURCE_DESC desc = GetNativeDesc(resource);

		GResource res;
		res.desc = resource;
		res.resource = heapCollection.CreateResource(device, desc, GetNativeState(resource.state), nullptr);
		res.address = res.resource->GetGPUVirtualAddress();

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
		rDesc.format = DXGI_FORMAT_UNKNOWN;
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
		rDesc.sizeInBytes = rDesc.width * rDesc.height * rDesc.strideInBytes;
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

		HeapCollectionType & heapCollection = GetCollection(ptr->desc.type);

		DX_API("Failed to release resource")
			heapCollection.ReleaseResource(ptr->resource);

		for(decltype(managedResources)::iterator it = managedResources.begin(); it != managedResources.end(); ++it) {
			if((&(*it)) == ptr) {
				managedResources.erase(it);
				return;
			}
		}

	}
}
