#pragma once

#include "Common.h"

namespace Netcode::Memory {

	template<typename T, typename NC_ALLOC_T>
	class StdAllocatorAdapter : public NC_ALLOC_T {
	public:
		using value_type = T;
		using pointer = T *;
		using const_pointer = const T *;

		using NC_ALLOC_T::NC_ALLOC_T;

		StdAllocatorAdapter(const NC_ALLOC_T & rhs) : NC_ALLOC_T{ rhs } { }

		pointer allocate(size_t n) {
			return static_cast<pointer>(NC_ALLOC_T::template Allocate<T>(n).Data());
		}

		void deallocate(void * p, size_t s) {
			NC_ALLOC_T::Deallocate(p, s);
		}

		template<typename ... U>
		inline void construct(pointer ptr, U && ... args) {
			MemoryBlock(ptr).Construct<T>(std::forward<U>(args)...);
		}

		inline void destroy(pointer ptr) {
			Destroy<T>(ptr);
		}

		template<typename U>
		struct rebind {
			using other = StdAllocatorAdapter<U, NC_ALLOC_T>;
		};
	};

}
