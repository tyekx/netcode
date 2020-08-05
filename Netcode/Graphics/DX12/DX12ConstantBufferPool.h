#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Decl.h"
#include <vector>
#include <list>
#include <memory>

namespace Netcode::Graphics::DX12 {

	class Resource;
	class HeapManager;

	/*
	Simple linear allocator designed to be reset every frame
	*/
	class ConstantBufferPool {

		struct CBufferAllocation;
		struct CBufferAllocationPage;
		struct CBufferPage;

		constexpr static size_t CBUFFER_PAGE_SIZE = 1 << 19;

		std::list<CBufferAllocationPage> allocationPages;
		std::vector<CBufferPage> pages;
		CBufferPage * currentPage;
		CBufferAllocationPage * currentAllocationPage;
		Ref<HeapManager> heapManager;
		uint8_t * mappedPtr;

		Ref<DX12::Resource> CreatePageResource();
		void ValidateCurrentPageFor(size_t size);
		void ValidateCurrentAllocationPage();

		ConstantBufferPool() = default;

	public:

		ConstantBufferPool(Ref<HeapManager> heapMan);
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
