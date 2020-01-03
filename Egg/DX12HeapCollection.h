#pragma once

#include "DX12Common.h"
#include <list>
#include <set>
#include <algorithm>

namespace Egg::Graphics::DX12 {

	struct HeapAlloc {

		com_ptr<ID3D12Heap> heap;

		struct ResourceAlloc {
			UINT64 sizeInBytes;
			UINT64 heapOffset;
			com_ptr<ID3D12Resource> resource;

			ResourceAlloc() = default;
			~ResourceAlloc() = default;

			ResourceAlloc(const ResourceAlloc &) = default;
			ResourceAlloc & operator=(const ResourceAlloc &) = default;

			ResourceAlloc(ResourceAlloc &&) = default;
			ResourceAlloc & operator=(ResourceAlloc &&) = default;

			ResourceAlloc(UINT64 size, UINT64 heapOffset, com_ptr<ID3D12Resource> res) : sizeInBytes{ size }, heapOffset{ heapOffset }, resource{ std::move(res) } { }

			bool operator<(const ResourceAlloc & ra) const {
				return sizeInBytes < ra.sizeInBytes;
			}
		};

		std::list<ResourceAlloc> used;
		std::multiset<ResourceAlloc> freed;

		D3D12_HEAP_TYPE type;

		UINT64 sizeInBytes;
		UINT64 usedInBytes;

		HeapAlloc() : heap{}, used{ }, freed{ }, type{ }, sizeInBytes{ 0 }, usedInBytes{ 0 } { }

		HeapAlloc(ID3D12Device * device, UINT64 sizeInBytes, D3D12_HEAP_TYPE type) : HeapAlloc() {
			this->sizeInBytes = sizeInBytes;
			this->type = type;

			D3D12_HEAP_DESC heapDesc = {};
			heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
			heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(type);
			heapDesc.SizeInBytes = sizeInBytes;

			DX_API("Failed to create heap")
				device->CreateHeap(&heapDesc, IID_PPV_ARGS(heap.GetAddressOf()));
		}

		HRESULT ReleaseResource(ID3D12Resource * resource) {
			auto it = std::find_if(std::begin(used), std::end(used), [resource](const ResourceAlloc & listItem) -> bool {
				return listItem.resource.Get() == resource;
			});

			if(it == std::end(used)) {
				return E_FAIL;
			}

			it->resource.Reset();
			freed.insert(*it);
			used.erase(it);

			return S_OK;
		}

		ID3D12Resource * CreateResource(ID3D12Device * device, const D3D12_RESOURCE_DESC & desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE * clearValue, const D3D12_RESOURCE_ALLOCATION_INFO & allocation) {
			com_ptr<ID3D12Resource> tempResource;

			/*
			partition point: returns the element for which the predicate returns false.
			Keeping it in a multiset enables us to have a sorted collection of freed allocations
			idx:  |   0    1    2     3  |
			pred: | true true true false |
			      |                   ^ this will be the partition point
			if no partiton point found end() will be returned
			*/
			decltype(freed)::iterator reuseableResource = std::partition_point(std::begin(freed), std::end(freed), [allocation](const ResourceAlloc & res) -> bool {
				return res.sizeInBytes < allocation.SizeInBytes;
			});

			UINT64 heapOffset;
			UINT64 allocationSize;

			if(reuseableResource == freed.end()) {
				heapOffset = usedInBytes;
				allocationSize = allocation.SizeInBytes;
			} else {
				heapOffset = reuseableResource->heapOffset;
				allocationSize = reuseableResource->sizeInBytes;
			}

			DX_API("Failed to create placed resource")
				device->CreatePlacedResource(heap.Get(), heapOffset, &desc, initState, clearValue, IID_PPV_ARGS(tempResource.GetAddressOf()));

			ID3D12Resource * rawPtr = tempResource.Get();

			used.emplace_back(allocationSize, heapOffset, std::move(tempResource));
			if(reuseableResource != freed.end()) {
				freed.erase(reuseableResource);
			}

			usedInBytes += allocationSize;

			return rawPtr;
		}

		bool CanFit(UINT64 sizeInBytes) const {
			UINT64 totalFreeSpace = (this->sizeInBytes - this->usedInBytes);

			if(totalFreeSpace < sizeInBytes) {
				return false;
			}

			for(const auto & iter : freed) {
				if(iter.sizeInBytes >= sizeInBytes) {
					return true;
				} else {
					totalFreeSpace -= iter.sizeInBytes;
				}
			}

			return totalFreeSpace < sizeInBytes;
		}
	};

	template<UINT64 GRANULARITY>
	class HeapCollection {
		using HeapsType = std::list<HeapAlloc>;

		void SortHeaps() {
			heaps.sort([](const HeapAlloc & lhs, const HeapAlloc & rhs) -> bool {
				return lhs.sizeInBytes > rhs.sizeInBytes;
			});
		}
	public:
		HeapsType heaps;

		D3D12_HEAP_TYPE  heapType;

		HeapCollection(D3D12_HEAP_TYPE heapType) : heapType{ heapType } { }

		ID3D12Resource * CreateResource(ID3D12Device * device, const D3D12_RESOURCE_DESC & desc, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE * clearValue) {
			const D3D12_RESOURCE_ALLOCATION_INFO alloc = device->GetResourceAllocationInfo(0, 1, &desc);
			
			ASSERT(GRANULARITY >= alloc.SizeInBytes, "Cant allocate %ull size while the granularity is just %ull", alloc.SizeInBytes, GRANULARITY);

			for(auto & heap : heaps) {
				if(heap.CanFit(alloc.SizeInBytes)) {
					return heap.CreateResource(device, desc, state, clearValue, alloc);
				}
			}

			auto & heap = heaps.emplace_back(device, GRANULARITY, heapType);

			ID3D12Resource * rawPtr = heap.CreateResource(device, desc, state, clearValue, alloc);

			SortHeaps();

			return rawPtr;
		}

		HRESULT ReleaseResource(ID3D12Resource * resource) {
			for(auto & heap : heaps) {
				if(SUCCEEDED(heap.ReleaseResource(resource))) {
					return S_OK;
				}
			}
			return E_FAIL;
		}
	};
}
