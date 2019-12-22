#include "DX12CbufferAllocator.h"

namespace Egg::Graphics::DX12 {

	inline HCBUFFER SetPageIndex(HCBUFFER handle, UINT16 pageIdx) {
		UINT64 pageIdxValue = pageIdx;
		pageIdxValue <<= 48;

		return (handle & 0x0000FFFFFFFFFFFFULL) |
		 (pageIdxValue & 0xFFFF000000000000ULL);
	}

	inline HCBUFFER SetSizeDiv256(HCBUFFER handle, UINT16 sizeDiv256) {
		UINT64 sizeDiv256Value = sizeDiv256;
		sizeDiv256Value <<= 32;

		return (handle & 0xFFFF0000FFFFFFFFULL) |
	  (sizeDiv256Value & 0x0000FFFF00000000ULL);
	}

	inline HCBUFFER SetByteOffset(HCBUFFER handle, UINT32 offset) {
		UINT64 offsetValue = offset;
		
		return (handle & 0xFFFFFFFF00000000ULL) |
		  (offsetValue & 0x00000000FFFFFFFFULL);
	}

	inline UINT32 GetByteOffset(HCBUFFER handle) {
		return static_cast<UINT32>(handle & 0x00000000FFFFFFFFULL);
	}

	inline UINT16 GetSizeDiv256(HCBUFFER handle) {
		return static_cast<UINT16>((handle & 0x0000FFFF00000000ULL) >> 32);
	}

	inline UINT16 GetPageIndex(HCBUFFER handle) {
		return static_cast<UINT16>((handle & 0xFFFF000000000000ULL) >> 48);
	}

	void CbufferAllocator::CbufferPage::CreateResources(ID3D12Device * device) {
		DX_API("Failed to create upload resource")
			device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
											D3D12_HEAP_FLAG_NONE,
											&CD3DX12_RESOURCE_DESC::Buffer(PAGE_SIZE),
											D3D12_RESOURCE_STATE_GENERIC_READ,
											nullptr,
											IID_PPV_ARGS(resource.GetAddressOf()));

		std::wstring resourceName = L"CBufferPage#" + std::to_wstring(1);

		DX_API("Failed to set name")
			resource->SetName(resourceName.c_str());

		CD3DX12_RANGE range{ 0,0 };

		void * tempPtr;

		DX_API("Failed to map cbuffer")
			resource->Map(0, &range, &tempPtr);

		mappedPtr = reinterpret_cast<BYTE *>(tempPtr);

		addr = resource->GetGPUVirtualAddress();
	}

	void CbufferAllocator::CbufferPage::ReleaseResources() {
		if(resource != nullptr) {
			CD3DX12_RANGE range{ 0,0 };
			resource->Unmap(0, &range);
			resource.Reset();
		}
	}

	CbufferAllocator::CbufferPage::~CbufferPage() {
		ReleaseResources();
	}

	bool CbufferAllocator::CbufferPage::CanHost(unsigned int alignedSize) {
		return (allocatedSize + alignedSize) <= PAGE_SIZE;
	}

	void CbufferAllocator::CbufferPage::Deallocate(HCBUFFER handle) {
		void * offsettedPointer = mappedPtr + GetByteOffset(handle);

		FreedItem * freedItem = reinterpret_cast<FreedItem *>(offsettedPointer);
		freedItem->byteoffset = GetByteOffset(handle);
		freedItem->sizeInBytes = GetSizeDiv256(handle) << 8;
		freedItem->nextItem = head;
		head = freedItem;
	}

	HCBUFFER CbufferAllocator::CbufferPage::Allocate(unsigned int alignedSize) {
		HCBUFFER handle = 0;
		handle = SetSizeDiv256(handle, alignedSize >> 8);
		handle = SetByteOffset(handle, allocatedSize);
		allocatedSize += alignedSize;
		return handle;
	}

	D3D12_GPU_VIRTUAL_ADDRESS CbufferAllocator::GetAddress(HCBUFFER handle) {
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

	void CbufferAllocator::CreateResources(ID3D12Device * dev) {
		device = dev;
		head = tail = nullptr;

		numPages = 1;
		head = new CbufferPage();
		tail = head;

		head->CreateResources(device);
	}

	void CbufferAllocator::ReleaseResources() {
		while(tail != nullptr) {
			CbufferPage * tempPrev = tail->prev;
			delete tail;
			tail = tempPrev;
		}
		tail = nullptr;
		head = nullptr;
		device = nullptr;
	}

	CbufferAllocator::~CbufferAllocator() {
		ReleaseResources();
	}

	void CbufferAllocator::AddRenderItemCbuffer(RenderItem * renderItem, HCBUFFER cbuffer, UINT slot) {
		ASSERT(renderItem->numCbuffers <= 8, "Too many constant buffers are attached to this item");

		renderItem->cbuffers[renderItem->numCbuffers].addr = GetAddress(cbuffer);
		renderItem->cbuffers[renderItem->numCbuffers].rootSigSlot = slot;

		renderItem->numCbuffers += 1;
	}

	void CbufferAllocator::SetRenderItemCbuffer(RenderItem * renderItem, HCBUFFER cbuffer, UINT idx, UINT slot) {
		ASSERT(idx < renderItem->numCbuffers && renderItem->numCbuffers <= 8, "Too many constant buffers are attached to this item");

		renderItem->cbuffers[idx].addr = GetAddress(cbuffer);
		renderItem->cbuffers[idx].rootSigSlot = slot;
	}

	void * CbufferAllocator::GetCbufferPointer(HCBUFFER handle) {
		CbufferPage * iter = nullptr;
		UINT16 i = 0;
		UINT16 pageIdx = GetPageIndex(handle);
		UINT32 byteOffset = GetByteOffset(handle);
		
		for(iter = head; iter != nullptr && i < pageIdx; iter = iter->next, ++i);

		ASSERT(iter != nullptr, "Page not found");

		return iter->mappedPtr + byteOffset;
	}

	HCBUFFER CbufferAllocator::AllocateCbuffer(unsigned int sizeInBytes) {
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

