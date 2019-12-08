#pragma once

#include "Common.h"
#include "HandleTypes.h"
#include "Utility.h"
#include "DX12RenderItem.h"

namespace Egg::Graphics::DX12 {

	class CbufferAllocator {
		// relatively big allocation for smaller allocation cost
		constexpr static UINT PAGE_SIZE = (1 << 16) * 64;

		// making sure before using byte magic @TODO: does this support endianness?
		static_assert(sizeof(HCBUFFER) == 8, "Your compiler does not seem to be agreeing on the size of ull");

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

			void Deallocate(HCBUFFER handle);

			HCBUFFER Allocate(unsigned int alignedSize);

		};

		CbufferPage * head;
		CbufferPage * tail;
		UINT16 numPages;

		ID3D12Device * device;

		D3D12_GPU_VIRTUAL_ADDRESS GetAddress(HCBUFFER handle);

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

		void * GetCbufferPointer(HCBUFFER handle);

		HCBUFFER AllocateCbuffer(unsigned int sizeInBytes);

	};

}
