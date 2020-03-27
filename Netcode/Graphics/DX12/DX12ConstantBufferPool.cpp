#include "DX12ConstantBufferPool.h"

namespace Netcode::Graphics::DX12 {

	ID3D12Resource * ConstantBufferPool::CreatePageResource() {
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
			currentPage->resource->Map(0, &readRange, reinterpret_cast<void **>(&mappedPtr));
		} else {
			if((CBUFFER_PAGE_SIZE - currentPage->offset) < size) {
				currentPage->resource->Unmap(0, nullptr);
				currentPage = &pages.emplace_back(CreatePageResource());
				currentPage->resource->Map(0, &readRange, reinterpret_cast<void **>(&mappedPtr));
			}
		}
	}

	void ConstantBufferPool::ValidateCurrentAllocationPage() {
		if(currentAllocationPage == nullptr ||
			currentAllocationPage->IsFull()) {
			currentAllocationPage = &allocationPages.emplace_back();
		}
	}

	void ConstantBufferPool::SetHeapManager(HeapManager * heapMan) {
		heapManager = heapMan;
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

		for(auto & page : pages) {
			heapManager->ReleaseResource(page.resource);
		}
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
		allocation->resource = currentPage->resource;
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

