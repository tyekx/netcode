#include "Common.h"
#include "../Exceptions.h"

namespace Netcode::Memory {

	void * AlignedMalloc(std::size_t numBytes, std::size_t alignment) {
		return _aligned_malloc(numBytes, alignment);
	}

	void AlignedFree(void * alignedPtr) {
		_aligned_free(alignedPtr);
	}

}
