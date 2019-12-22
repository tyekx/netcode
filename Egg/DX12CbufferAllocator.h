#pragma once

#include "HandleTypes.h"
#include "Utility.h"
#include "DX12Common.h"
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

			void CreateResources(ID3D12Device * device);

			void ReleaseResources();

			~CbufferPage();

			bool CanHost(unsigned int alignedSize);

			void Deallocate(HCBUFFER handle);

			HCBUFFER Allocate(unsigned int alignedSize);

		};

		CbufferPage * head;
		CbufferPage * tail;
		UINT16 numPages;

		ID3D12Device * device;

		D3D12_GPU_VIRTUAL_ADDRESS GetAddress(HCBUFFER handle);

	public:

		void CreateResources(ID3D12Device * dev);

		void ReleaseResources();

		~CbufferAllocator();

		void AddRenderItemCbuffer(RenderItem * renderItem, HCBUFFER cbuffer, UINT slot);

		void SetRenderItemCbuffer(RenderItem * renderItem, HCBUFFER cbuffer, UINT idx, UINT slot);

		void * GetCbufferPointer(HCBUFFER handle);

		HCBUFFER AllocateCbuffer(unsigned int sizeInBytes);

	};

}
