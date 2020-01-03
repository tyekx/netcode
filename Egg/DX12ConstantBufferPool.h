#pragma once

#include "DX12ResourceDesc.h"
#include "DX12HeapCollection.h"

#include <deque>
#include <array>

namespace Egg::Graphics::DX12 {

	/*
	Simple linear allocator designed to be reset every frame
	*/
	class ConstantBufferPool {

		// actual cbuffer page with GPU resource
		struct CBufferPage {
			ID3D12Resource * resource;
			UINT64 offset;
			const D3D12_GPU_VIRTUAL_ADDRESS baseAddr;

			CBufferPage(ID3D12Resource * resource) : resource{ resource }, offset{ 0 }, baseAddr{ resource->GetGPUVirtualAddress() } { }
		};

		// a single allocation for a cbuffer 32 bytes of management data
		struct CBufferAllocation {
			D3D12_GPU_VIRTUAL_ADDRESS address;
			ID3D12Resource * resource;
			UINT64 sizeInBytes;
			UINT64 locationOffset;
		};

		/*
		A bulk of allocations, does not invalidate the pointers to a CBufferAllocation object,
		while maintaining the dynamically expanding nature
		*/
		struct CBufferAllocationPage {
			constexpr static UINT PAGE_SIZE = 1024;
			UINT nextId;
			std::array<CBufferAllocation, PAGE_SIZE> allocations;

			CBufferAllocation * Insert() {
				return allocations.data() + nextId++;
			}

			bool IsFull() const {
				return nextId == PAGE_SIZE;
			}
		};

		constexpr static UINT64 ALLOCATION_GRANULARITY = 1 << 24; // 16MB

		HeapCollection<ALLOCATION_GRANULARITY> cbufferHeaps;

		std::deque<CBufferAllocationPage> allocationPages;

		std::vector<CBufferPage> pages;

		CBufferPage * currentPage;

		CBufferAllocationPage * currentAllocationPage;

		ID3D12Device * device;

		ID3D12Resource * CreatePageResource() {
			D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(ALLOCATION_GRANULARITY, D3D12_RESOURCE_FLAG_NONE);

			return cbufferHeaps.CreateResource(device, desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);
		}

		void ValidateCurrentPageFor(size_t size) {
			ASSERT(size <= ALLOCATION_GRANULARITY, "Size is too big to allocate");

			if(currentPage == nullptr ||
			  ((ALLOCATION_GRANULARITY - currentPage->offset) < size)) {
				currentPage = &pages.emplace_back(CreatePageResource());
			}
		}

		void ValidateCurrentAllocationPage() {
			if(currentAllocationPage == nullptr ||
				currentAllocationPage->IsFull()) {
				currentAllocationPage = &allocationPages.emplace_back();
			}
		}

	public:

		void SetDevice(ID3D12Device * device) {
			this->device = device;
		}

		ConstantBufferPool() : cbufferHeaps{ D3D12_HEAP_TYPE_UPLOAD }, allocationPages{}, pages{},
			currentPage{ nullptr }, currentAllocationPage{ nullptr }, device{ nullptr } {
			
		}

		~ConstantBufferPool() {
			Reset();
		}

		/*
		Clears all allocations, resets the ownership
		*/
		void Reset() {
			allocationPages.clear();
			currentAllocationPage = nullptr;
			currentPage = nullptr;

			for(auto & page : pages) {
				cbufferHeaps.ReleaseResource(page.resource);
			}
			pages.clear();
		}

		/*
		Allocates the desired amount of bytes
		*/
		uint64_t CreateConstantBuffer(size_t size) {
			ASSERT(size != 0, "Cant allocate 0 bytes");
			
			ValidateCurrentPageFor(size);
			ValidateCurrentAllocationPage();

			constexpr UINT64 alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1;
			
			size_t alignedSize = (size + alignment) & (~alignment);

			CBufferAllocation * allocation = currentAllocationPage->Insert();
			allocation->resource = currentPage->resource;
			allocation->locationOffset = currentPage->offset;
			allocation->address = currentPage->baseAddr + currentPage->offset;
			allocation->sizeInBytes = alignedSize;

			currentPage->offset += alignedSize;

			return reinterpret_cast<uint64_t>( allocation );
		}

		D3D12_GPU_VIRTUAL_ADDRESS GetNativeHandle(uint64_t handle) const {
			return reinterpret_cast<CBufferAllocation *>(handle)->address;
		}

		void CopyData(uint64_t handle, const void * srcData, size_t srcDataSizeInBytes) {
			CBufferAllocation * alloc = reinterpret_cast<CBufferAllocation *>(handle);
			uint8_t * destPtr = nullptr;
			const D3D12_RANGE readRange = CD3DX12_RANGE{ 0, 0 };
			const D3D12_RANGE writtenRange = CD3DX12_RANGE{ alloc->locationOffset, alloc->locationOffset + alloc->sizeInBytes };

			DX_API("Failed to map ptr")
				alloc->resource->Map(0, &readRange, reinterpret_cast<void **>(&destPtr));

			memcpy((destPtr + alloc->locationOffset), srcData, srcDataSizeInBytes);

			alloc->resource->Unmap(0, &writtenRange);
		}

	};

}
