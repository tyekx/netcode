#pragma once

#include "Common.h"

namespace Netcode::Memory {

	template<size_t ALIGNMENT = 16>
	class AlignedMallocator {
	public:
		template<typename T>
		static MemoryBlock Allocate(size_t numElements) {
			return AlignedMalloc(numElements * sizeof(T), ALIGNMENT);
		}

		static inline void Deallocate(void * p, size_t s) {
			AlignedFree(p);
		}

		AlignedMallocator() noexcept = default;
	};

}
