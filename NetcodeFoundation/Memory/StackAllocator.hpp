#pragma once

#include "Common.h"
#include "StdAllocatorProxy.hpp"
#include "ResourceAllocator.hpp"

namespace Netcode::Memory {

	class StackAllocator
	{
		struct Resource {
			uint8_t * data;
			size_t offset;
			size_t stackSize;
			size_t alignment;

			Resource() noexcept = default;
		};

		std::shared_ptr<Resource> resource;
	public:
		StackAllocator(size_t stackSize, size_t alignment = 16) : resource{ } {
			size_t headSize;
			StdAllocatorProxy<Resource, ResourceAllocator> resourceAlloc{ stackSize, alignment, &headSize };
			resource = std::allocate_shared<Resource>(resourceAlloc);
			resource->data = reinterpret_cast<uint8_t *>(resource.get()) + headSize;
			resource->stackSize = stackSize;
			resource->alignment = alignment;
		}

		using AllocType = typename StackAllocator;

		template<typename V>
		using DeleterType = AllocatorDeleter<V, AllocType>;

		template<typename V>
		using UniquePtrType = std::unique_ptr<V, DeleterType<V>>;


		StackAllocator(const AllocType & rhs) = default;
		StackAllocator & operator=(const AllocType &) = default;
		StackAllocator(AllocType &&) noexcept = default;
		StackAllocator & operator=(AllocType &&) noexcept = default;

		template<typename T, typename ... U>
		inline std::shared_ptr<T> MakeShared(U && ... args) {
			return std::allocate_shared<T, StdAllocatorProxy<T, AllocType>>(*this, std::forward<U>(args)...);
		}

		template<typename T, typename ... U>
		inline UniquePtrType<T> MakeUnique(U && ... args) {
			return UniquePtrType<T>(Allocate<T>(1).Construct<U...>(std::forward<U>(args)...), DeleterType<T>(*this));
		}

		template<typename T>
		MemoryBlock Allocate(size_t numElements) {
			std::size_t allocBytes = Align<size_t>(numElements * sizeof(T), resource->alignment);

			Detail::OutOfRangeAssertion((resource->offset + allocBytes) <= resource->stackSize);

			void * p = resource->data + resource->offset;

			resource->offset += allocBytes;

			return p;
		}

		template<typename T, typename ... U>
		inline T * Construct(U && ... args) {
			return Allocate<T>(1).Construct<U...>(std::forward<U>(args)...);
		}

		void Deallocate(void * p, size_t s) { }
	};


}
