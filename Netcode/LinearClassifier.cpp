#include "LinearClassifier.h"
#include <cstdlib>

namespace Netcode::Memory {

	LinearClassifier::LinearClassifier() {
		Clear();
	}

	void LinearClassifier::Clear() noexcept {
		allocation = nullptr;
		ptr = nullptr;
		nextByte = 0;
	}

	LinearClassifier::LinearClassifier(unsigned int sizeInBytes) {
		Initialize(sizeInBytes);
	}

	LinearClassifier::~LinearClassifier() {
		std::free(allocation);
		Clear();
	}

	void LinearClassifier::Initialize(unsigned int sizeInBytes) {
		if(allocation == nullptr) {
			allocation = std::malloc(sizeInBytes);
			ptr = (unsigned char *)allocation;
			nextByte = 0;
		}
	}

	void * LinearClassifier::Detach() {
		void * tmp = allocation;
		Clear();
		return tmp;
	}

	void * LinearClassifier::GetBasePointer() {
		return allocation;
	}

	void * LinearClassifier::Allocate(unsigned int n) {
		void * p = ptr + nextByte;
		nextByte += n;
		return p;
	}

}
