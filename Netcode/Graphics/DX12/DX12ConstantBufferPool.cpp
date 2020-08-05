#include "DX12ConstantBufferPool.h"
#include <Netcode/Common.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include "DX12Includes.h"
#include "DX12Resource.h"
#include "DX12HeapManager.h"
#include <array>

namespace Netcode::Graphics::DX12 {

	// actual cbuffer page with GPU resource
	struct ConstantBufferPool::CBufferPage {
		Ref<DX12::Resource> resource;
		UINT64 offset;
		const D3D12_GPU_VIRTUAL_ADDRESS baseAddr;

		CBufferPage(Ref<DX12::Resource> resource) : resource{ resource }, offset{ 0 }, baseAddr{ resource->resource->GetGPUVirtualAddress() } { }
	};

	// a single allocation for a cbuffer 32 bytes of management data
	struct ConstantBufferPool::CBufferAllocation {
		D3D12_GPU_VIRTUAL_ADDRESS address;
		ID3D12Resource * resource;
		UINT64 sizeInBytes;
		UINT64 locationOffset;
	};

	/*
	A bulk of allocations, does not invalidate the pointers to a CBufferAllocation object,
	while maintaining the dynamically expanding nature
	*/
	struct ConstantBufferPool::CBufferAllocationPage {
		constexpr static UINT PAGE_SIZE = 1024;
		UINT nextId;
		std::array<ConstantBufferPool::CBufferAllocation, PAGE_SIZE> allocations;

		ConstantBufferPool::CBufferAllocation * Insert() {
			return allocations.data() + nextId++;
		}

		bool IsFull() const {
			return nextId == PAGE_SIZE;
		}
	};


	Ref<DX12::Resource> ConstantBufferPool::CreatePageResource() {
		ResourceDesc desc;
		desc.depth = 1;
		desc.dimension = ResourceDimension::BUFFER;
		desc.state = ResourceState::ANY_READ;
		desc.type = ResourceType::PERMANENT_UPLOAD;
		desc.flags = ResourceFlags::NONE;
		desc.sizeInBytes = CBUFFER_PAGE_SIZE;
		desc.format = DXGI_FORMAT_UNKNOWN;
		desc.mipLevels = 1;
		desc.height = 1;
		desc.strideInBytes = 1;
		desc.width = CBUFFER_PAGE_SIZE;
		return heapManager->CreateResource(desc);
	}

	void ConstantBufferPool::ValidateCurrentPageFor(size_t size) {
		const CD3DX12_RANGE readRange{ 0,0 };

		if(currentPage == nullptr) {
			currentPage = &pages.emplace_back(CreatePageResource());
			currentPage->resource->resource->Map(0, &readRange, reinterpret_cast<void **>(&mappedPtr));
		} else {
			if((CBUFFER_PAGE_SIZE - currentPage->offset) < size) {
				currentPage->resource->resource->Unmap(0, nullptr);
				currentPage = &pages.emplace_back(CreatePageResource());
				currentPage->resource->resource->Map(0, &readRange, reinterpret_cast<void **>(&mappedPtr));
			}
		}
	}

	void ConstantBufferPool::ValidateCurrentAllocationPage() {
		if(currentAllocationPage == nullptr ||
			currentAllocationPage->IsFull()) {
			currentAllocationPage = &allocationPages.emplace_back();
		}
	}

	ConstantBufferPool::ConstantBufferPool(Ref<HeapManager> heapMan) : ConstantBufferPool{} {
		heapManager = std::move(heapMan);
	}

	ConstantBufferPool::~ConstantBufferPool() {
		Reset();
	}

	/*
	Clears all allocations, resets the ownership
	*/

	void ConstantBufferPool::Reset() {
		allocationPages.clear();
		currentAllocationPage = nullptr;
		currentPage = nullptr;
		pages.clear();
	}

	/*
	Clears only allocations
	*/

	void ConstantBufferPool::Clear() {
		allocationPages.clear();

		for(auto & page : pages) {
			page.offset = 0;
		}
		currentAllocationPage = nullptr;

		if(pages.empty()) {
			currentPage = nullptr;
		} else {
			currentPage = &pages.front();
		}
	}

	/*
	Allocates the desired amount of bytes
	*/

	uint64_t ConstantBufferPool::CreateConstantBuffer(size_t size) {
		ASSERT(size != 0, "Cant allocate 0 bytes");

		ValidateCurrentPageFor(size);
		ValidateCurrentAllocationPage();

		constexpr UINT64 alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1;

		size_t alignedSize = (size + alignment) & (~alignment);

		CBufferAllocation * allocation = currentAllocationPage->Insert();
		allocation->resource = currentPage->resource->resource.Get();
		allocation->locationOffset = currentPage->offset;
		allocation->address = currentPage->baseAddr + currentPage->offset;
		allocation->sizeInBytes = alignedSize;

		currentPage->offset += alignedSize;

		return reinterpret_cast<uint64_t>(allocation);
	}

	D3D12_GPU_VIRTUAL_ADDRESS ConstantBufferPool::GetNativeHandle(uint64_t handle) const {
		return reinterpret_cast<CBufferAllocation *>(handle)->address;
	}

	void ConstantBufferPool::CopyData(uint64_t handle, const void * srcData, size_t srcDataSizeInBytes) {
		CBufferAllocation * alloc = reinterpret_cast<CBufferAllocation *>(handle);

		memcpy((mappedPtr + alloc->locationOffset), srcData, srcDataSizeInBytes);
	}

}

