#pragma once

#include "Common.h"

namespace Netcode::Memory {
	/*
	Utility "allocator", instead of allocating,
	throws an exception with the requested allocation size
	*/
	template<typename BASE>
	class SpyAllocator : public BASE {
	public:
		using BASE::BASE;

		SpyAllocator(const BASE & rhs) : BASE{ rhs } { }

		template<typename T>
		MemoryBlock Allocate(size_t n) {
			throw size_t{ sizeof(T) * n };
		}

		void Deallocate(void * p, size_t s) {
			assert(p == nullptr);
		}
	};

}
