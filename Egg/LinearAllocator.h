#pragma once

#include <cstdint>
#include <cstdlib>

namespace Egg {

	template<size_t SIZE_IN_BYTES>
	class LinearAllocator {
		uint8_t * bytes;
		size_t numBytes;

	public:
		LinearAllocator() : bytes(static_cast<uint8_t *>(std::malloc(SIZE_IN_BYTES))), numBytes{ 0 } { }

		~LinearAllocator() noexcept {
			std::free(bytes);
			bytes = nullptr;
			numBytes = 0;
		}

		template<typename T>
		T * Allocate() {
			T * ptr = reinterpret_cast<T *>(bytes + numBytes);
			numBytes += sizeof(T);
			new (ptr) T();
			return ptr;
		}

		template<typename T>
		void Deallocate(T * ptr) {
			ptr->~T();
		}
	};

}
