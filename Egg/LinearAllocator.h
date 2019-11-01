#pragma once

namespace Egg::Memory {

	/*
	Linear allocator class which helps filling pre-allocated memory. Calling the constructors / destructors is the task of the user
	If not detached, the linear allocator will delete the memory at destruction
	*/
	class LinearAllocator {
		void * allocation;
		unsigned char * ptr;
		unsigned int nextByte;

		void Clear() noexcept;

	public:
		LinearAllocator();

		LinearAllocator(unsigned int sizeInBytes);

		~LinearAllocator();

		void Initialize(unsigned int sizeInBytes);

		/*
		Detaching the allocation base pointer from the instance. This pointer must be deallocated using std::free.
		*/
		void * Detach();

		/*
		Gets the allocation base pointer from the instance. Without delegating the ownership of the allocation
		*/
		void * GetBasePointer();

		void * Allocate(unsigned int n);

		template<typename T>
		T * Allocate() {
			void * p = ptr + nextByte;
			nextByte += sizeof(T);
			return reinterpret_cast<T *>(p);
		}
	};

}
