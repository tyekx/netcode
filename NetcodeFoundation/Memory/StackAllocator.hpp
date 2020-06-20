#pragma once

#include "Common.h"
#include "StdAllocatorAdapter.hpp"
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
		StackAllocator(size_t stackSize = 65536, size_t alignment = 16) : resource{ } {
			size_t headSize;
			StdAllocatorAdapter<Resource, ResourceAllocator> resourceAlloc{ ResourceAllocator{ stackSize, alignment, &headSize } };
			resource = std::allocate_shared<Resource>(resourceAlloc);
			resource->data = reinterpret_cast<uint8_t *>(resource.get()) + headSize;
			resource->stackSize = stackSize;
			resource->alignment = alignment;
		}

		using AllocType = StackAllocator;

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
			return std::allocate_shared<T, StdAllocatorAdapter<T, AllocType>>(*this, std::forward<U>(args)...);
		}

		template<typename T, typename ... U>
		inline UniquePtrType<T> MakeUnique(U && ... args) {
			return UniquePtrType<T>(Allocate<T>(1).Construct<U...>(std::forward<U>(args)...), DeleterType<T>(*this));
		}

		template<typename T>
		MemoryBlock Allocate(size_t numElements) {
			std::size_t allocBytes = Align<size_t>(numElements * sizeof(T), resource->alignment);

			OutOfRangeAssertion((resource->offset + allocBytes) <= resource->stackSize);

			void * p = resource->data + resource->offset;

			resource->offset += allocBytes;

			return p;
		}

		template<typename T, typename ... U>
		inline T * Construct(U && ... args) {
			return Allocate<T>(1).Construct<U...>(std::forward<U>(args)...);
		}

		template<typename T>
		void Deallocate(T * p, size_t s) { }

		/*
		Cheap but dangerous call, resets all allocations, does not invoke destructors
		*/
		void Reset() {
			resource->offset = 0;
		}
	};


}
