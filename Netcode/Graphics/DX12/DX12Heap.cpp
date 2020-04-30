#include "DX12Heap.h"

namespace Netcode::Graphics::DX12 {

	Heap::Heap(com_ptr<ID3D12Device> dev, uint64_t sizeInBytes, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags) :
		sizeInBytes{ sizeInBytes },
		offsetInBytes{ 0 },
		freeSizeInBytes{ 0 },
		type{ type },
		device{ std::move(dev) },
		heap{ nullptr },
		freedResources{ } {

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Flags = flags;
		heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(type);
		heapDesc.SizeInBytes = sizeInBytes;

		DX_API("Failed to create heap")
			device->CreateHeap(&heapDesc, IID_PPV_ARGS(heap.GetAddressOf()));
	}

	void Heap::ReturnResource(Resource * rawPtr) {
		std::unique_ptr<Resource> wrappedPtr{ rawPtr };
		freeSizeInBytes += wrappedPtr->sizeInBytes;
		wrappedPtr->resource.Reset();
		wrappedPtr->heap.reset();
		freedResources.emplace_back(std::move(wrappedPtr));
	}

	bool Heap::HasEnoughSpace(uint64_t bytesToStore) {
		const uint64_t totalFreeSpace = sizeInBytes - offsetInBytes;

		if(totalFreeSpace >= bytesToStore) {
			return true;
		}

		if(freeSizeInBytes < bytesToStore) {
			return false;
		}

		auto it = std::find_if(std::begin(freedResources), std::end(freedResources), [bytesToStore](std::unique_ptr<Resource> & resource) -> bool {
			return resource->sizeInBytes >= bytesToStore;
		});

		return it != std::end(freedResources);
	}

	std::shared_ptr<Resource> Heap::CreateResource(const ResourceDesc & resourceDesc, const D3D12_RESOURCE_DESC & desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE * clearValue, const D3D12_RESOURCE_ALLOCATION_INFO & allocation) {
		const uint64_t bytesToStore = allocation.SizeInBytes;

		auto it = std::find_if(std::begin(freedResources), std::end(freedResources), [bytesToStore](std::unique_ptr<Resource> & resource) -> bool {
			return resource->sizeInBytes >= bytesToStore;
		});

		std::shared_ptr<Resource> resource;
		com_ptr<ID3D12Resource> dx12Resource;
		uint64_t allocationSize;
		uint64_t heapOffset;

		if(it != std::end(freedResources)) {
			resource = std::move((*it));
			allocationSize = resource->sizeInBytes;
			heapOffset = resource->heapOffset;
			freedResources.erase(it);
			freeSizeInBytes -= allocationSize;
		} else {
			allocationSize = allocation.SizeInBytes;
			heapOffset = offsetInBytes;
			offsetInBytes += allocationSize;
		}

		device->CreatePlacedResource(heap.Get(), heapOffset, &desc, initState, clearValue, IID_PPV_ARGS(dx12Resource.GetAddressOf()));

		if(resource == nullptr) {
			resource = std::shared_ptr<Resource>(
				new Resource{ allocationSize, heapOffset, resourceDesc, desc, std::move(dx12Resource), shared_from_this() },
				std::bind(&Heap::ReturnResource, this, std::placeholders::_1));
		} else {
			resource->resource = std::move(dx12Resource);
			resource->desc = resourceDesc;
			resource->nativeDesc = desc;
			resource->heap = shared_from_this();
		}

		return resource;
	}

}

