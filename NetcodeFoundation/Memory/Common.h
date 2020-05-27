#pragma once

#include <cstdint>
#include <memory>

template<typename T> struct is_shared_ptr : std::false_type {};

template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

namespace Netcode::Memory {

	namespace Detail {

		void OutOfRangeAssertion(bool isInRange);

		void UndefinedBehaviourAssertion(bool isDefinedBehaviour);

	}

	void * AlignedMalloc(std::size_t numBytes, std::size_t alignment);
	void AlignedFree(void * alignedPtr);

	template<typename INT_T>
	inline INT_T Align(INT_T numBytes, INT_T alignment) {
		return (numBytes + (alignment - 1)) & (~(alignment - 1));
	}

	template<typename T>
	void * Destroy(T * ptr) {
		using W = typename std::remove_cv<T>::type;
		ptr->~W();
		return ptr;
	}

	class MemoryBlock {
		void * ptr;
	public:
		MemoryBlock(void * p) noexcept : ptr{ p } { }

		inline void * Data() {
			return ptr;
		}

		// @TODO: add noexcept conditional
		template<typename T, typename ... U>
		inline T * Construct(U && ... args) {
			T * typedPtr = static_cast<T *>(ptr);
			new (typedPtr) T(std::forward<T>(args)...);
			return typedPtr;
		}
	};

	template<typename T, typename NC_ALLOC_T>
	class AllocatorDeleter : public NC_ALLOC_T {
	public:
		using NC_ALLOC_T::NC_ALLOC_T;

		AllocatorDeleter(const NC_ALLOC_T & rhs) : NC_ALLOC_T{ rhs } { }

		void operator()(T * ptr) {
			NC_ALLOC_T::Deallocate(Destroy<T>(ptr), 1);
		}
	};

}
