#pragma once

#include "Exception/ArithmeticException.h"
#include "Exception/IOException.h"
#include "Exception/NotSupportedException.h"
#include "Exception/OutOfRangeException.h"
#include "Exception/UndefinedBehaviourException.h"

namespace Netcode {

	void OutOfMemoryAssertion(bool isValidPtr);

	void OutOfRangeAssertion(bool isInRange);

	void UndefinedBehaviourAssertion(bool isDefinedBehaviour);

}
