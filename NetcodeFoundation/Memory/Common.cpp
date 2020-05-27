#include "Common.h"
#include "../Exceptions.h"

namespace Netcode::Memory {

	namespace Detail {

		void OutOfRangeAssertion(bool isInRange)
		{
			if(!isInRange) {
				throw OutOfRangeException{ "Allocation out of range" };
			}
		}

		void UndefinedBehaviourAssertion(bool isDefinedBehaviour) {
			if(!isDefinedBehaviour) {
				throw UndefinedBehaviourException{ "The program would run into an undefined state" };
			}
		}

	}

	void * AlignedMalloc(std::size_t numBytes, std::size_t alignment) {
		return _aligned_malloc(numBytes, alignment);
	}

	void AlignedFree(void * alignedPtr) {
		_aligned_free(alignedPtr);
	}

}
