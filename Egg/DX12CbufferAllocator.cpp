#include "DX12CbufferAllocator.h"

namespace Egg {

	HCBUFFER SetPageIndex(HCBUFFER handle, UINT16 pageIdx) {
		UINT64 pageIdxValue = pageIdx;
		pageIdxValue <<= 48;

		return (handle & 0x0000FFFFFFFFFFFFULL) |
		 (pageIdxValue & 0xFFFF000000000000ULL);
	}

	HCBUFFER SetSizeDiv256(HCBUFFER handle, UINT16 sizeDiv256) {
		UINT64 sizeDiv256Value = sizeDiv256;
		sizeDiv256Value <<= 32;

		return (handle & 0xFFFF0000FFFFFFFFULL) |
	  (sizeDiv256Value & 0x0000FFFF00000000ULL);
	}

	HCBUFFER SetByteOffset(HCBUFFER handle, UINT32 offset) {
		UINT64 offsetValue = offset;
		
		return (handle & 0xFFFFFFFF00000000ULL) |
		  (offsetValue & 0x00000000FFFFFFFFULL);
	}

	UINT32 GetByteOffset(HCBUFFER handle) {
		return static_cast<UINT32>(handle & 0x00000000FFFFFFFFULL);
	}

	UINT16 GetSizeDiv256(HCBUFFER handle) {
		return static_cast<UINT16>((handle & 0x0000FFFF00000000ULL) >> 32);
	}

	UINT16 GetPageIndex(HCBUFFER handle) {
		return static_cast<UINT16>((handle & 0xFFFF000000000000ULL) >> 48);
	}



	void Graphics::DX12::CbufferAllocator::CbufferPage::Deallocate(HCBUFFER handle) {
		void * offsettedPointer = mappedPtr + GetByteOffset(handle);

		FreedItem * freedItem = reinterpret_cast<FreedItem *>(offsettedPointer);
		freedItem->byteoffset = GetByteOffset(handle);
		freedItem->sizeInBytes = GetSizeDiv256(handle) << 8;
		freedItem->nextItem = head;
		head = freedItem;
	}

	HCBUFFER Graphics::DX12::CbufferAllocator::CbufferPage::Allocate(unsigned int alignedSize) {
		HCBUFFER handle = 0;
		handle = SetSizeDiv256(handle, alignedSize >> 8);
		handle = SetByteOffset(handle, allocatedSize);
		allocatedSize += alignedSize;
		return handle;
	}

	D3D12_GPU_VIRTUAL_ADDRESS Graphics::DX12::CbufferAllocator::GetAddress(HCBUFFER handle) {
		CbufferPage * iter = nullptr;
		UINT16 i = 0;
		UINT16 pageIdx = GetPageIndex(handle);
		UINT32 byteOffset = GetByteOffset(handle);

		ASSERT(byteOffset % 256 == 0, "sanity check failed: byte offset is not 256 aligned");
		
		//@TODO: if it matters this could be optimized

		for(iter = head; iter != nullptr && i < pageIdx; iter = iter->next, ++i);

		ASSERT(iter != nullptr, "Page not found");

		return iter->addr + byteOffset;
	}

	void * Graphics::DX12::CbufferAllocator::GetCbufferPointer(HCBUFFER handle) {
		CbufferPage * iter = nullptr;
		UINT16 i = 0;
		UINT16 pageIdx = GetPageIndex(handle);
		UINT32 byteOffset = GetByteOffset(handle);
		
		for(iter = head; iter != nullptr && i < pageIdx; iter = iter->next, ++i);

		ASSERT(iter != nullptr, "Page not found");

		return iter->mappedPtr + byteOffset;
	}

	HCBUFFER Graphics::DX12::CbufferAllocator::AllocateCbuffer(unsigned int sizeInBytes) {
		unsigned int aligned = Egg::Utility::Align256(sizeInBytes);

		ASSERT(tail != nullptr && head != nullptr, "Pointers are unset, forgot to call CreateResources?");
		ASSERT(aligned <= PAGE_SIZE, "Cant allocate this big of a cbuffer");
		UINT16 idx = numPages;

		for(CbufferPage * iter = tail; iter != nullptr; iter = iter->prev) {
			--idx;
			if(iter->CanHost(aligned)) {
				return SetPageIndex(iter->Allocate(aligned), idx);
			}
		}

		CbufferPage * page = new CbufferPage();
		page->prev = tail;
		page->next = nullptr;
		tail->next = page;
		tail = page;
		++numPages;

		return SetPageIndex(tail->Allocate(aligned), idx);
	}
}

