#pragma once

#include "Common.h"

namespace Netcode::Memory {

	/*
	Detail allocator for other allocators
	*/
	class ResourceAllocator {
		size_t totalSize;
		size_t alignment;
		size_t * headSize;
	public:
		ResourceAllocator(size_t totalSize, size_t alignment, size_t * headSize) :
			totalSize{ totalSize }, alignment{ alignment }, headSize{ headSize } {}

		template<typename T>
		MemoryBlock Allocate(size_t n) {
			assert(n == 1);
			(*headSize) = Align<size_t>(n * sizeof(T), alignment);
			return AlignedMalloc((*headSize) + totalSize, alignment);
		}

		static void Deallocate(void * ptr, size_t n) {
			AlignedFree(ptr);
		}
	};

}
