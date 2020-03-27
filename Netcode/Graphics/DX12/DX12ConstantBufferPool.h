#pragma once

#include "DX12ResourceDesc.h"
#include "DX12HeapCollection.h"

#include <deque>
#include <array>

namespace Netcode::Graphics::DX12 {

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

		HeapManager * heapManager;

		uint8_t * mappedPtr;

		ID3D12Resource * CreatePageResource();

		void ValidateCurrentPageFor(size_t size);

		void ValidateCurrentAllocationPage();

	public:

		void SetHeapManager(HeapManager * heapMan);

		~ConstantBufferPool();

		/*
		Clears all allocations, resets the ownership
		*/
		void Reset();

		/*
		Clears only allocations
		*/
		void Clear();

		/*
		Allocates the desired amount of bytes
		*/
		uint64_t CreateConstantBuffer(size_t size);

		D3D12_GPU_VIRTUAL_ADDRESS GetNativeHandle(uint64_t handle) const;

		void CopyData(uint64_t handle, const void * srcData, size_t srcDataSizeInBytes);

	};

}
