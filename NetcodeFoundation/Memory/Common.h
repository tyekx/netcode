#pragma once

#include <cstdint>
#include <memory>

template<typename T> struct is_shared_ptr : std::false_type {};

template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

namespace Netcode::Memory {

	namespace Detail {

		void OutOfMemoryAssertion(bool isValidPtr);

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

	template<typename FROM, typename TO, typename ... U>
	TO * Reconstruct(FROM ** ppData, U && ... args) {
		if constexpr(!std::is_same<FROM, void>::value) {
			(*ppData)->~FROM();
		}
		TO * pRawMem = reinterpret_cast<TO *>(*ppData);
		*ppData = nullptr;
		return new (pRawMem) TO{ std::forward<U>(args)... };
	}

	template<typename T>
	T * Relocate(T * srcPtr, size_t numBytes) {
		T * dstPtr = reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(srcPtr) + numBytes);
		new (dstPtr) T{ static_cast<const T &>(*srcPtr) };
		srcPtr->~T();
		return dstPtr;
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
			new (typedPtr) T ( std::forward<U>(args)... );
			return typedPtr;
		}

		bool operator!=(std::nullptr_t) const {
			return !operator==(nullptr);
		}

		bool operator==(std::nullptr_t) const {
			return ptr == nullptr;
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
