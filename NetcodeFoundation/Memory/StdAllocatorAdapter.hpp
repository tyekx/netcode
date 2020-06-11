#pragma once

#include "Common.h"

namespace Netcode::Memory {

	template<typename T, typename NC_ALLOC_T>
	class StdAllocatorAdapter : public NC_ALLOC_T {
	public:
		using value_type = T;
		using pointer = T *;
		using const_pointer = const T *;

		StdAllocatorAdapter() : NC_ALLOC_T{} { }
		StdAllocatorAdapter(const NC_ALLOC_T & rhs) : NC_ALLOC_T{ rhs } { }
		StdAllocatorAdapter(NC_ALLOC_T && rhs) noexcept : NC_ALLOC_T{ std::move(rhs) } { }
		StdAllocatorAdapter(StdAllocatorAdapter &&) noexcept = default;
		StdAllocatorAdapter(const StdAllocatorAdapter &) = default;

		bool operator==(const StdAllocatorAdapter<T, NC_ALLOC_T> & rhs) const {
			return NC_ALLOC_T::operator==(static_cast<const NC_ALLOC_T&>(rhs));
		}

		pointer allocate(size_t n) {
			return static_cast<pointer>(NC_ALLOC_T::template Allocate<T>(n).Data());
		}

		void deallocate(T * p, size_t s) {
			NC_ALLOC_T::template Deallocate<T>(p, s);
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
