#include "LinearAllocator.h"
#include <cstdlib>

namespace Egg::Memory {

	LinearAllocator::LinearAllocator() {
		Clear();
	}

	void LinearAllocator::Clear() noexcept {
		allocation = nullptr;
		ptr = nullptr;
		nextByte = 0;
	}

	LinearAllocator::LinearAllocator(unsigned int sizeInBytes) {
		Initialize(sizeInBytes);
	}

	LinearAllocator::~LinearAllocator() {
		std::free(allocation);
		Clear();
	}

	void LinearAllocator::Initialize(unsigned int sizeInBytes) {
		if(allocation == nullptr) {
			allocation = std::malloc(sizeInBytes);
			ptr = (unsigned char *)allocation;
			nextByte = 0;
		}
	}

	void * LinearAllocator::Detach() {
		void * tmp = allocation;
		Clear();
		return tmp;
	}

	void * LinearAllocator::GetBasePointer() {
		return allocation;
	}

	void * LinearAllocator::Allocate(unsigned int n) {
		void * p = ptr + nextByte;
		nextByte += n;
		return p;
	}

}
