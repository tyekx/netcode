#include "Exceptions.h"

namespace Netcode {

	void OutOfMemoryAssertion(bool isValidPtr) {
		if(!isValidPtr) {
			throw OutOfMemoryException{ "Out of memory" };
		}
	}

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

	void NotImplementedAssertion(const char * text)
	{
		throw NotImplementedException{ text };
	}

}
