#pragma once

#include "Common.h"
#include "ResourceAllocator.hpp"
#include "SpyAllocator.hpp"
#include "StdAllocatorProxy.hpp"

namespace Netcode::Memory {

	template<typename T>
	class PoolAllocator {
	private:
		struct FreeListItem {
			FreeListItem * next;

			FreeListItem(FreeListItem * n) : next{ n } { }

			~FreeListItem() noexcept {
				next = nullptr;
			}
		};

		struct Resource {
			uint8_t * ptr;
			size_t offset;
			size_t stride;
			size_t numBytes;
			size_t alignment;
			FreeListItem * head;

			Resource() noexcept = default;
		};

		std::shared_ptr<Resource> resource;

	protected:
		template<typename T>
		MemoryBlock Allocate(size_t numElements) {
			// changing stride for a pool allocator is a hard no
			Detail::UndefinedBehaviourAssertion(resource->stride == sizeof(T));

			if(resource->head != nullptr) {
				FreeListItem * freeListItem = resource->head;
				resource->head = freeListItem->next;
				freeListItem->~FreeListItem();

				T * tPtr = reinterpret_cast<T *>(freeListItem);
				return tPtr;
			} else {
				// ran out of pre-allocated memory
				Detail::OutOfRangeAssertion((resource->offset + sizeof(T) * numElements) <= resource->numBytes);

				T * tPtr = reinterpret_cast<T *>(resource->ptr + resource->offset);
				resource->offset += sizeof(T) * numElements;
				return tPtr;
			}
		}
	public:
		using AllocType = typename PoolAllocator<T>;

		PoolAllocator(size_t poolSize, size_t alignment) {
			size_t stride = -1;
			size_t poolSizeInBytes = -1;

			using SpyType = SpyAllocator<AllocType>;

			/*
			For future me: in case of a T being a form of shared_ptr<U>, we are forced to somehow figure out the stride prior to
			allocating the continous memory. This was done by invoking the allocate shared, but instead of allocating, we
			throw the requested size as an exception. According to the docs, the allocate_shared function requests memory only once,
			making it suitable for this hackery.
			*/

			if constexpr(is_shared_ptr<T>::value) {
				try {
					T sPtr = std::allocate_shared<T::element_type, StdAllocatorProxy<T::element_type, SpyType>>(*this);

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
			resource = std::allocate_shared<Resource, StdAllocatorProxy<Resource, ResourceAllocator>>(poolAlloc);
			resource->ptr = reinterpret_cast<uint8_t *>(resource.get()) + headSize;
			resource->numBytes = poolSizeInBytes;
			resource->stride = stride;
			resource->alignment = alignment;
		}

		/*
		if T is a shared_ptr<W>, then we return by value and construct by std::allocate_shared
		*/
		template<typename ... U>
		typename std::conditional<is_shared_ptr<T>::value, T, T *>::type Construct(U && ... args) {
			using Proxy = StdAllocatorProxy< T, AllocType >;
			// since T is a shared_ptr<W>, it'll have a T::element_type
			if constexpr(is_shared_ptr<T>::value) {
				return std::allocate_shared< T::element_type, Proxy, U...>(
					Proxy{ *this },
					std::forward<U>(args)...
					);
			} else {
				return Allocate<T>(1).Construct<T, U...>(std::forward<U>(args)...);
			}
		}

		void Deallocate(void * p, size_t s) {
			FreeListItem * ptr = reinterpret_cast<FreeListItem *>(p);
			new (ptr) FreeListItem(resource->head);
			resource->head = ptr;
		}
	};
}
