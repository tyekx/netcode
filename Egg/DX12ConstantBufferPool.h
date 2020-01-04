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

		constexpr static size_t CBUFFER_PAGE_SIZE = 1 << 19;

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

		std::deque<CBufferAllocationPage> allocationPages;

		std::vector<CBufferPage> pages;

		CBufferPage * currentPage;

		CBufferAllocationPage * currentAllocationPage;

		ID3D12Device * device;

		HeapManager * heapManager;

		ID3D12Resource * CreatePageResource() {
			ResourceDesc desc;
			desc.depth = 1;
			desc.dimension = ResourceDimension::BUFFER;
			desc.state = ResourceState::GENERIC_READ;
			desc.type = ResourceType::TRANSIENT_UPLOAD;
			desc.flags = ResourceFlags::NONE;
			desc.sizeInBytes = CBUFFER_PAGE_SIZE;
			desc.format = DXGI_FORMAT_UNKNOWN;
			desc.mipLevels = 1;
			desc.height = 1;
			desc.strideInBytes = 1;
			desc.width = CBUFFER_PAGE_SIZE;
			return heapManager->CreateResource(desc);
		}

		void ValidateCurrentPageFor(size_t size) {
			if(currentPage == nullptr ||
			  ((CBUFFER_PAGE_SIZE - currentPage->offset) < size)) {
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

		void SetHeapManager(HeapManager * heapMan) {
			heapManager = heapMan;
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
				heapManager->ReleaseResource(page.resource);
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
