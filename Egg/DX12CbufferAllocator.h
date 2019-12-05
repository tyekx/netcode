#pragma once

#include "Common.h"
#include "HandleTypes.h"
#include "Utility.h"
#include "DX12RenderItem.h"

namespace Egg::Graphics::DX12 {

	class CbufferAllocator {
		// relatively big allocation for smaller allocation cost
		constexpr static UINT PAGE_SIZE = (1 << 16) * 64;

		struct PackedHandle {
			UINT16 pageIdx;
			UINT16 sizeDiv256;
			UINT32 byteOffset;
		};

		// making sure before using byte magic @TODO: does this support endianness?
		static_assert(sizeof(HCBUFFER) == 8, "Your compiler does not seem to be agreeing on the size of ull");
		static_assert(sizeof(PackedHandle) == 8, "Your compiler does not seem to pack the struct properly");

		struct CbufferPage {
			/*
			because the smallest allocatable size is 256 bytes, we can reuse that memory for other purposes.
			@TODO: write a better allocator where we dont write mapped pointers and upload these data to gpu.
			@TODO: actually use the freelist
			*/
			struct FreedItem {
				UINT byteoffset;
				UINT sizeInBytes;
				FreedItem * nextItem;
			};

			FreedItem * head;
			com_ptr<ID3D12Resource> resource;
			D3D12_GPU_VIRTUAL_ADDRESS addr;
			BYTE * mappedPtr;
			UINT allocatedSize;
			CbufferPage * next;
			CbufferPage * prev;

			void CreateResources(ID3D12Device * device) {
				DX_API("Failed to create upload resource")
					device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
													D3D12_HEAP_FLAG_NONE,
													&CD3DX12_RESOURCE_DESC::Buffer(PAGE_SIZE),
													D3D12_RESOURCE_STATE_GENERIC_READ,
													nullptr,
													IID_PPV_ARGS(resource.GetAddressOf()));

				CD3DX12_RANGE range{ 0,0 };
				
				void * tempPtr;

				DX_API("Failed to map cbuffer")
					resource->Map(0, &range, &tempPtr);

				mappedPtr = reinterpret_cast<BYTE *>(tempPtr);

				addr = resource->GetGPUVirtualAddress();
			}

			void ReleaseResources() {
				if(resource != nullptr) {
					CD3DX12_RANGE range{ 0,0 };
					resource->Unmap(0, &range);
					resource.Reset();
				}
			}

			~CbufferPage() {
				ReleaseResources();
			}

			bool CanHost(unsigned int alignedSize) {
				return (allocatedSize + alignedSize) <= PAGE_SIZE;
			}

			void Deallocate(HCBUFFER handle) {
				PackedHandle * ph = reinterpret_cast<PackedHandle *>(&handle);
				void * offsettedPointer = mappedPtr + ph->byteOffset;
				
				FreedItem * freedItem = reinterpret_cast<FreedItem *>(offsettedPointer);
				freedItem->byteoffset = ph->byteOffset;
				freedItem->sizeInBytes = ph->sizeDiv256 << 8;

				freedItem->nextItem = head;
				head = freedItem;
			}

			HCBUFFER Allocate(unsigned int alignedSize) {
				PackedHandle ph;
				ph.byteOffset = allocatedSize;
				ph.pageIdx = 0;
				ph.sizeDiv256 = alignedSize >> 8;
				allocatedSize += alignedSize;
				return *(reinterpret_cast<HCBUFFER *>(&ph));
			}

		};

		inline HCBUFFER SetPageIdx(HCBUFFER handle, UINT16 pageIdx) {
			PackedHandle * ph = reinterpret_cast<PackedHandle *>(&handle);
			ph->pageIdx = pageIdx;
			return *(reinterpret_cast<HCBUFFER *>(ph));
		}

		CbufferPage * head;
		CbufferPage * tail;
		UINT16 numPages;

		ID3D12Device * device;

		D3D12_GPU_VIRTUAL_ADDRESS GetAddress(HCBUFFER handle) {
			CbufferPage * iter = nullptr;
			UINT16 i = 0;
			PackedHandle * ph = reinterpret_cast<PackedHandle *>(&handle);
			for(iter = head; iter != nullptr && i < ph->pageIdx; iter = iter->next, ++i);

			ASSERT(ph->byteOffset % 256 != 0, "sanity check failed: byte offset is not 256 aligned");
			ASSERT(iter != nullptr, "Page not found");

			return iter->addr + ph->byteOffset;
		}

	public:

		void CreateResources(ID3D12Device * dev) {
			device = dev;
			head = tail = nullptr;

			numPages = 1;
			head = new CbufferPage();
			tail = head;

			head->CreateResources(device);
		}

		void ReleaseResources() {
			while(tail != nullptr) {
				CbufferPage * tempPrev = tail->prev;
				delete tail;
				tail = tempPrev;
			}
			tail = nullptr;
			head = nullptr;
			device = nullptr;
		}

		~CbufferAllocator() {
			ReleaseResources();
		}

		void AddRenderItemCbuffer(RenderItem * renderItem, HCBUFFER cbuffer, UINT slot) {
			ASSERT(renderItem->numCbuffers <= 8, "Too many constant buffers are attached to this item");

			renderItem->cbuffers[renderItem->numCbuffers].addr = GetAddress(cbuffer);
			renderItem->cbuffers[renderItem->numCbuffers].rootSigSlot = slot;

			renderItem->numCbuffers += 1;
		}

		void SetRenderItemCbuffer(RenderItem * renderItem, HCBUFFER cbuffer, UINT idx, UINT slot) {
			ASSERT(idx < renderItem->numCbuffers && renderItem->numCbuffers <= 8, "Too many constant buffers are attached to this item");

			renderItem->cbuffers[idx].addr = GetAddress(cbuffer);
			renderItem->cbuffers[idx].rootSigSlot = slot;
		}

		void * GetCbufferPointer(HCBUFFER handle) {
			CbufferPage * iter = nullptr;
			UINT16 i = 0;
			PackedHandle * ph = reinterpret_cast<PackedHandle *>(&handle);
			for(iter = head; iter != nullptr && i < ph->pageIdx; iter = iter->next, ++i);

			ASSERT(iter != nullptr, "Page not found");

			return iter->mappedPtr + ph->byteOffset;
		}

		HCBUFFER AllocateCbuffer(unsigned int sizeInBytes) {
			unsigned int aligned = Egg::Utility::Align256(sizeInBytes);

			ASSERT(tail != nullptr && head != nullptr, "Pointers are unset, forgot to call CreateResources?");
			ASSERT(aligned <= PAGE_SIZE, "Cant allocate this big of a cbuffer");
			UINT16 idx = numPages;

			for(CbufferPage * iter = tail; iter != nullptr; iter = iter->prev) {
				--idx;
				if(iter->CanHost(aligned)) {
					return SetPageIdx(iter->Allocate(aligned), idx);
				}
			}

			CbufferPage * page = new CbufferPage();
			page->prev = tail;
			page->next = nullptr;
			tail->next = page;
			tail = page;
			++numPages;

			return SetPageIdx(tail->Allocate(aligned), idx);
		}

	};

}
