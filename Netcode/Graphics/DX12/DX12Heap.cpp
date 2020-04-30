#include "DX12Heap.h"

namespace Netcode::Graphics::DX12 {

	Heap::Heap(com_ptr<ID3D12Device> dev, uint64_t sizeInBytes, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags) :
		sizeInBytes{ sizeInBytes },
		offsetInBytes{ 0 },
		freedSizeInBytes{ 0 },
		type{ type },
		flags{ flags },
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

	bool Heap::IsCompatible(D3D12_HEAP_TYPE requiredHeap, D3D12_HEAP_FLAGS requiredFlags) const
	{
		bool b = requiredHeap == type && ((flags & requiredFlags) == requiredFlags);

		if(!b) {
			Log::Debug("[GPU] Not compatible");
		}

		return b;
	}

	void Heap::ReturnResource(Resource * rawPtr) {
		std::unique_ptr<Resource> wrappedPtr{ rawPtr };
		wrappedPtr->resource.Reset();
		wrappedPtr->heap.reset();

		const uint64_t freeSize = freedSizeInBytes + wrappedPtr->sizeInBytes;
		if(offsetInBytes == freeSize) {
			freedResources.clear();
			offsetInBytes = 0;
			freedSizeInBytes = 0;
			Log::Debug("Clearing");
		} else if(offsetInBytes == (wrappedPtr->heapOffset + wrappedPtr->sizeInBytes)) {
			offsetInBytes -= wrappedPtr->sizeInBytes;
		} else {
			freedResources.emplace_back(std::move(wrappedPtr));
			freedSizeInBytes = freeSize;
		}
	}

	Resource * Heap::AllocateResource(uint64_t sizeInBytes)
	{
		return nullptr;
	}

	bool Heap::HasEnoughSpace(uint64_t bytesToStore) const {
		const uint64_t unallocatedSize = GetUnallocatedSize();

		if(unallocatedSize >= bytesToStore) {
			return true;
		}

		bool b = false;

		for(auto & i : freedResources) {
			if(i->sizeInBytes >= bytesToStore) {
				b = true;
				break;
			}
		}

		if(!b) {
			Log::Debug("[GPU] Does not have enough free space");
		}

		return b;
	}

	uint64_t Heap::GetUnallocatedSize() const {
		return sizeInBytes - offsetInBytes;
	}

	void Heap::Defragment()
	{
		// sort freed resources by heapOffset ASCENDING
		std::sort(std::begin(freedResources), std::end(freedResources), [](const std::unique_ptr<Resource> & lhs, const std::unique_ptr<Resource> & rhs) ->bool {
			return lhs->heapOffset < rhs->heapOffset;
		});

		// remove freed resources from the end
		while(!freedResources.empty() &&
			  (freedResources.back()->heapOffset + freedResources.back()->sizeInBytes) == offsetInBytes) {
			offsetInBytes -= freedResources.back()->sizeInBytes;
			freedResources.pop_back();
		}

		// check if we are done
		if(freedResources.empty()) {
			return;
		}

		// merge consecutive resources together
		size_t lhs = 0;
		size_t rhs = 1;
		size_t loopCount = freedResources.size();

		while(rhs < loopCount) {
			std::unique_ptr<Resource> & lRes = freedResources.at(lhs);
			std::unique_ptr<Resource> & rRes = freedResources.at(rhs);

			// mergeable?
			if((lRes->heapOffset + lRes->sizeInBytes) == rRes->heapOffset) {
				lRes->sizeInBytes += rRes->sizeInBytes;
				rRes.reset();
			} else {
				lhs = rhs;
			}

			rhs += 1;
		}

		// clean nullptrs
		std::vector<std::unique_ptr<Resource>> defragmented;
		defragmented.reserve(freedResources.size());

		for(auto & i : freedResources) {
			if(i != nullptr) {
				defragmented.emplace_back(std::move(i));
			}
		}

		freedResources = std::move(defragmented);
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
			resource = DX12ResourceRef { std::move((*it)) };
			allocationSize = resource->sizeInBytes;
			heapOffset = resource->heapOffset;
			freedResources.erase(it);
			freedSizeInBytes -= allocationSize;
		} else {
			allocationSize = allocation.SizeInBytes;
			heapOffset = offsetInBytes;
			offsetInBytes += allocationSize;
		}

		DX_API("Failed to create placed resource")
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

		debug.push_back(resource);

		return resource;
	}

}

