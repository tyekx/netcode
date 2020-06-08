#pragma once

#include "Common.h"
#include "ResourceAllocator.hpp"
#include "SpyAllocator.hpp"
#include "StdAllocatorAdapter.hpp"

namespace Netcode::Memory {

	namespace Detail {

		template<size_t TYPE_SIZE>
		struct FreeListSelectIntType {
			using type = void;
		};

		template<>
		struct FreeListSelectIntType<4> {
			using type = uint32_t;
		};

		template<>
		struct FreeListSelectIntType<2> {
			using type = uint16_t;
		};

		template<>
		struct FreeListSelectIntType<1> {
			using type = uint8_t;
		};

		template<size_t X, size_t Y>
		struct FreeListCondition {
			constexpr static bool value = X <= Y;
		};

		template<typename T>
		struct FreeListNullValue {
			constexpr static T value = std::numeric_limits<T>::max();
		};

		template<typename T>
		struct FreeListNullValue<T *> {
			constexpr static T * value = nullptr;
		};

		template<typename T, typename U>
		struct FreeListContentType {
			using type = typename std::conditional< FreeListCondition<sizeof(U *), sizeof(U)>::value, T *, typename FreeListSelectIntType<sizeof(U)>::type >::type;
		};

	}

	/*
	Allocator type for allocating T-s. Has a specialization for std::shared_ptr allocations.
	Only allocates a single contiguous memory block with the given alignment 
	Can deallocate items, the deallocated items will be chained into a freelist.
	If sizeof(T) is less than sizeof(void*) then the maximum allocated size can only be
	sizeof(T) bytes of integer's max value - 1. Eg: sizeof(short) then the maximum pool size can
	only be std::numeric_limits<uint16_t>::max()
	*/
	template<typename T>
	class PoolAllocator {
	private:
		struct FreeListItem {
			using ContentType = typename Detail::FreeListContentType<FreeListItem, T>::type;
			constexpr static ContentType NullValue = Detail::FreeListNullValue<ContentType>::value;

			ContentType next;

			FreeListItem(ContentType n) : next{ n } { }

			~FreeListItem() noexcept {
				next = static_cast<ContentType>(0);
			}
		};

		using FreeListItemRef = typename FreeListItem::ContentType;

		struct Resource {
			uint8_t * ptr;
			size_t offset;
			size_t stride;
			size_t numBytes;
			size_t alignment;
			FreeListItemRef head;

			Resource() noexcept = default;
		};

		inline FreeListItem * PointerOf(FreeListItemRef index) {
			if constexpr(std::is_integral<FreeListItemRef>::value) {
				// if its an integral type, then we need to do an indirection
				return reinterpret_cast<FreeListItem *>(reinterpret_cast<FreeListItemRef *>(resource->ptr) + index);
			} else {
				static_assert(std::is_same<FreeListItem *, FreeListItemRef>::value, "Type mismatch");
				return index;
			}
		}

		inline FreeListItemRef IndexOf(FreeListItem * ptr) {
			if constexpr(std::is_integral<FreeListItemRef>::value) {
				// if its an integral type, then we need to do an indirection
				return static_cast<FreeListItemRef>(reinterpret_cast<FreeListItemRef *>(ptr) - reinterpret_cast<FreeListItemRef *>(resource->ptr));
			} else {
				static_assert(std::is_same<FreeListItem *, FreeListItemRef>::value, "Type mismatch");
				return ptr;
			}
		}

		std::shared_ptr<Resource> resource;

	protected:

		template<typename U>
		MemoryBlock Allocate(size_t numElements) {
			// changing stride for a pool allocator is a hard no
			Detail::UndefinedBehaviourAssertion(resource->stride == sizeof(U));
			// no support for arrays
			Detail::UndefinedBehaviourAssertion(numElements == 1);

			if(resource->head != FreeListItem::NullValue) {
				FreeListItem * freeListItem = PointerOf(resource->head);
				resource->head = freeListItem->next;
				freeListItem->~FreeListItem();

				U * tPtr = reinterpret_cast<U *>(freeListItem);
				return tPtr;
			} else {
				// ran out of pre-allocated memory
				Detail::OutOfRangeAssertion((resource->offset + sizeof(U) * numElements) <= resource->numBytes);

				U * tPtr = reinterpret_cast<U *>(resource->ptr + resource->offset);
				resource->offset += sizeof(U) * numElements;
				return tPtr;
			}
		}

	public:
		using AllocType = PoolAllocator<T>;

		PoolAllocator(size_t poolSize, size_t alignment) {
			size_t stride = -1;
			size_t poolSizeInBytes = -1;

			using SpyType = SpyAllocator<AllocType>;

			if constexpr(std::is_integral<FreeListItemRef>::value) {
				Detail::UndefinedBehaviourAssertion(poolSize < FreeListItem::NullValue);
			}

			/*
			For future me: in case of a T being a form of shared_ptr<U>, we are forced to somehow figure out the stride prior to
			allocating the continous memory. This was done by invoking the allocate shared, but instead of allocating, we
			throw the requested size as an exception. According to the docs, the allocate_shared function requests memory only once,
			making it suitable for this hackery.
			*/

			if constexpr(is_shared_ptr<T>::value) {
				try {
					T sPtr = std::allocate_shared<typename T::element_type, StdAllocatorProxy<typename T::element_type, SpyType>>(*this);

					// always undefined behaviour if the previous call does not throw
					Detail::UndefinedBehaviourAssertion(false);
				} catch(size_t s) {
					stride = s;
				}

				poolSizeInBytes = Align<size_t>(poolSize * stride, alignment);
			} else {
				stride = sizeof(T);
				poolSizeInBytes = Align<size_t>(poolSize * stride, alignment);
			}

			Detail::UndefinedBehaviourAssertion(stride % alignment == 0);

			size_t headSize;
			ResourceAllocator poolAlloc{ poolSizeInBytes, alignment, &headSize };
			resource = std::allocate_shared<Resource, StdAllocatorAdapter<Resource, ResourceAllocator>>(poolAlloc);
			resource->ptr = reinterpret_cast<uint8_t *>(resource.get()) + headSize;
			resource->numBytes = poolSizeInBytes;
			resource->stride = stride;
			resource->alignment = alignment;
			resource->head = FreeListItem::NullValue;
		}

		/*
		if T is a shared_ptr<W>, then we return by value and construct by std::allocate_shared
		*/
		template<typename ... U>
		typename std::conditional<is_shared_ptr<T>::value, T, T *>::type Construct(U && ... args) {
			using Proxy = StdAllocatorAdapter<T, AllocType>;
			// since T is a shared_ptr<W>, it'll have a T::element_type
			if constexpr(is_shared_ptr<T>::value) {
				return std::allocate_shared<typename T::element_type, Proxy, U...>(
					Proxy{ *this },
					std::forward<U>(args)...
					);
			} else {
				return Allocate<T>(1).Construct<T, U...>(std::forward<U>(args)...);
			}
		}

		void Deallocate(void * p, size_t s) {
			Detail::UndefinedBehaviourAssertion(s == 1);

			FreeListItem * ptr = reinterpret_cast<FreeListItem *>(p);
			new (ptr) FreeListItem(resource->head);
			resource->head = IndexOf(ptr);
		}

		/*
		Cheap but dangerous call, resets all allocations, does not invoke destructors
		Because of this, it is only allowed when T is trivially destructible
		*/
		void Reset() {
			if constexpr(!is_shared_ptr<T>::value) {
				resource->offset = 0;
				resource->head = nullptr;
			} else {
				Detail::UndefinedBehaviourAssertion(false);
			}
		}
	};
}
