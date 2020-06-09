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
			Detail::UndefinedBehaviourAssertion(n == 1);

			(*headSize) = Align<size_t>(sizeof(T), alignment);

			void * p = AlignedMalloc((*headSize) + totalSize, alignment);

			Detail::OutOfMemoryAssertion(p != nullptr);

			return p;
		}

		template<typename T>
		static void Deallocate(T * ptr, size_t n) {
			AlignedFree(ptr);
		}
	};

}
